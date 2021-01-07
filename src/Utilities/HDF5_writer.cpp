//
// Created by Vetle Wegner Ingeberg on 01/10/2020.
//

#include <map>
#include <string>
#include <exception>

#include <cerrno>

#include "Utilities/HDF5_writer.h"

#include <spdlog/spdlog.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <Parameters/experimentsetup.h>

const std::map<std::string, hid_t> entry_map =
        {{"dtype", H5T_NATIVE_UINT16},
         {"id", H5T_NATIVE_UINT16},
         {"adcdata", H5T_NATIVE_UINT16},
         {"cfddata", H5T_NATIVE_UINT16},
         {"timestamp", H5T_NATIVE_INT64},
         {"cfdcorr", H5T_IEEE_F64LE},
         {"energy", H5T_IEEE_F64LE},
         {"cfdfail", H5T_NATIVE_HBOOL},
         {"finishcode", H5T_NATIVE_HBOOL}
        };

haddr_t Create_column(hid_t &group, const char *column_name, hid_t type, int no_rows, int i)
{
    hsize_t dims[1];
    herr_t status;
    dims[0] = no_rows;
    hid_t space = H5Screate_simple(1, dims, NULL);
    hid_t dcpl = H5Pcreate (H5P_DATASET_CREATE);
    H5Pset_layout (dcpl, H5D_CONTIGUOUS); // compact allows us the memory map the file
    H5Pset_alloc_time(dcpl, H5D_ALLOC_TIME_EARLY); // need this to allocate the space so offset exists
    hid_t dset = H5Dcreate(group, column_name, type, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    haddr_t offset = H5Dget_offset(dset);
    H5D_space_status_t space_status;
    H5Dget_space_status(dset, &space_status);

    spdlog::info("offset[{:d}] = {:x} allocated: {}\n", i, (unsigned int)offset,
                         (space_status == H5D_SPACE_STATUS_ALLOCATED ? "yes" : "no"));

    status = H5Dclose (dset);
    status = H5Pclose (dcpl);
    status = H5Sclose (space);

    return offset;
}

void HDF5_Writer::allocate(const char *fname, const int &rows)
{
    noRows = rows;
    hid_t file;    /* Handles */
    herr_t status;
    haddr_t	offsets[MAX_COLUMNS];
    hsize_t dims[1];

    file = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    hid_t group = H5Gcreate1(file, "data", 0);

    // Since this is C++ and there are no such thing as reflection we will do this manually.
    int i = 0;
    for (auto &column : entry_map){
        offsets[i] = Create_column(group, column.first.c_str(), column.second, noRows, i);
        ++i;
    }

    H5Gclose(group);
    status = H5Fclose(file);

    struct stat s{};
    status = stat(fname, &s);
    if ( status < 0 )
        throw std::runtime_error(std::string("stat ") + fname + " failed: " + strerror(errno));
    spdlog::info("File size {}", (unsigned long long)s.st_size);

    fd = open(fname, O_RDWR);
    if ( fd < 0 )
        throw std::runtime_error(std::string("open ") + fname + " failed: " + strerror(errno));

    char *mapped = (char *)mmap(NULL, s.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if ( mapped == MAP_FAILED )
        throw std::runtime_error(std::string("mmap ") + fname + " failed: " + strerror(errno));

    points.setup(mapped, offsets);
}

HDF5_Writer::HDF5_Writer(const char *fname, const int &rows) : noRows( 0 ), row_idx( 0 ), fd( 0 )
{
    allocate(fname, rows);
}

HDF5_Writer::~HDF5_Writer()
{
    points.reset();
    close(fd);
}

bool HDF5_Writer::Write(const Parser::Entry_t &entry)
{
    if (row_idx >= noRows)
        return false;
    auto dinfo = GetDetector(entry.address);
    points.dtype[row_idx] = dinfo.type;
    if (dinfo.type == clover)
        points.id[row_idx] = NUM_CLOVER_DETECTORS*dinfo.detectorNum+dinfo.telNum;
    else
        points.id[row_idx] = dinfo.detectorNum;
    points.adcdata[row_idx] = entry.adcdata;
    points.cfddata[row_idx] = entry.cfddata;
    points.timestamp[row_idx] = entry.timestamp;
    points.cfdcorr[row_idx] = entry.cfdcorr;
    points.energy[row_idx] = entry.energy;
    points.cfdfail[row_idx] = entry.cfdfail;
    points.finishcode[row_idx++] = entry.finishcode;
    return true;
}