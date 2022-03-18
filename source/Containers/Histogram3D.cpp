//
// Created by Vetle Wegner Ingeberg on 02/03/2022.
//

#include "Histogram3D.h"

#include <iostream>

#ifdef H3D_USE_BUFFER
const unsigned int Histogram3D::buffer_max;
#endif /* H2D_USE_BUFFER */

// ########################################################################

Histogram3D::Histogram3D( const std::string& name, const std::string& title,
                          Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xt,
                          Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& yt,
                          Axis::index_t ch3, Axis::bin_t l3, Axis::bin_t r3, const std::string& zt,
                          const std::string& path)
        : Named( name, title, path )
        , xaxis( name+"_xaxis", ch1, l1, r1, xt )
        , yaxis( name+"_yaxis", ch2, l2, r2, yt )
        , zaxis( name+"_zaxis", ch3, l3, r3, zt)
        , entries( 0 )
#ifndef USE_ROWS
        , data( nullptr )
#else
        , rows( nullptr )
#endif
{
#ifdef H3D_USE_BUFFER
    buffer.reserve(buffer_max);
#endif /* H2D_USE_BUFFER */

#ifndef USE_ROWS
    data = new data_t[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zaxis.GetBinCountAll()];
#else
    rows = new data_t**[zaxis.GetBinCountAll()];
    for(Axis::index_t z=0; z<zaxis.GetBinCountAll(); ++z) {
        rows[z] = new data_t *[yaxis.GetBinCountAll()];
        for (Axis::index_t y = 0; y < yaxis.GetBinCountAll(); ++y)
            rows[z][y] = new data_t[xaxis.GetBinCountAll()];
    }
#endif
    Reset();
}

// ########################################################################

Histogram3D::~Histogram3D()
{
#ifndef USE_ROWS
    delete data;
#else
    for(int z=0; z<zaxis.GetBinCountAll(); ++z) {
        for (int y = 0; y < yaxis.GetBinCountAll(); ++y)
            delete rows[z][y];
        delete rows[z];
    }
    delete rows;
#endif
}

// ########################################################################

void Histogram3D::Add(const Histogram3Dp &other, data_t scale)
{
    if( !other
        || other->GetName() != GetName()
        || other->GetAxisX().GetLeft() != xaxis.GetLeft()
        || other->GetAxisX().GetRight() != xaxis.GetRight()
        || other->GetAxisX().GetBinCount() != xaxis.GetBinCount()
        || other->GetAxisY().GetLeft() != yaxis.GetLeft()
        || other->GetAxisY().GetRight() != yaxis.GetRight()
        || other->GetAxisY().GetBinCount() != yaxis.GetBinCount()
        || other->GetAxisZ().GetLeft() != zaxis.GetLeft()
        || other->GetAxisZ().GetRight() != zaxis.GetRight()
        || other->GetAxisZ().GetBinCount() != zaxis.GetBinCount() )
        return;

#ifdef H3D_USE_BUFFER
    other->FlushBuffer();
    FlushBuffer();
#endif /* H3D_USE_BUFFER */

#ifndef USE_ROWS
    for(Axis::index_t i=0; i<xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zaxis.GetBinCountAll(); ++i)
        data[i] += scale * other->data[i];
#else
    for(Axis::index_t z=0; z<zaxis.GetBinCountAll(); ++z )
        for(Axis::index_t y=0; y<yaxis.GetBinCountAll(); ++y )
            for(Axis::index_t x=0; x<xaxis.GetBinCountAll(); ++x )
            rows[z][y][x] += scale*other->rows[z][y][x];
#endif
    // Update total count
    entries += scale * other->entries;
}

// ########################################################################

Histogram3D::data_t Histogram3D::GetBinContent(Axis::index_t xbin, Axis::index_t ybin, Axis::index_t zbin)
{
#ifdef H3D_USE_BUFFER
    if( !buffer.empty() )
        FlushBuffer();
#endif /* H3D_USE_BUFFER */

    if( xbin>=0 && xbin<xaxis.GetBinCountAll() &&
        ybin>=0 && ybin<yaxis.GetBinCountAll() &&
        zbin>=0 && zbin<zaxis.GetBinCountAll() ) {
#ifndef USE_ROWS
        return data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin +
                    xaxis.GetBinCountAll()*ybin + xbin];
#else
        return rows[zbin][ybin][xbin];
#endif // USE_ROWS
    } else
        return 0;
}

// ########################################################################

void Histogram3D::SetBinContent(Axis::index_t xbin, Axis::index_t ybin, Axis::index_t zbin, data_t c)
{
#ifdef H3D_USE_BUFFER
    if( !buffer.empty() )
        FlushBuffer();
#endif /* H3D_USE_BUFFER */

    if( xbin>=0 && xbin<xaxis.GetBinCountAll() &&
        ybin>=0 && ybin<yaxis.GetBinCountAll() &&
        zbin>=0 && zbin<zaxis.GetBinCountAll() ) {
#ifndef USE_ROWS
        data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin +
             xaxis.GetBinCountAll()*ybin + xbin] = c;
#else
        rows[zbin][ybin][xbin] = c;
#endif // USE_ROWS
    }
}

// ########################################################################

void Histogram3D::FillDirect(Axis::bin_t x, Axis::bin_t y, Axis::bin_t z, data_t weight)
{
    const int xbin = xaxis.FindBin( x );
    const int ybin = yaxis.FindBin( y );
    const int zbin = zaxis.FindBin( z );
#ifndef USE_ROWS
    data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin +
         xaxis.GetBinCountAll()*ybin + xbin] += weight;
#else
    rows[zbin][ybin][xbin] += weight;
    entries += 1;
#endif // USE_ROWS
}

// ########################################################################

#ifdef H3D_USE_BUFFER
void Histogram3D::FlushBuffer()
{
    if( !buffer.empty() ) {
      for ( auto &v : buffer )
        FillDirect(v.x, v.y, v.z, v.w);
      buffer.clear();
    }
}
#endif /* H3D_USE_BUFFER */

// ########################################################################

void Histogram3D::Reset()
{
#ifdef H2D_USE_BUFFER
    buffer.clear();
#endif /* H2D_USE_BUFFER */
    for(Axis::index_t z=0; z<zaxis.GetBinCountAll(); ++z )
        for(Axis::index_t y=0; y<yaxis.GetBinCountAll(); ++y )
            for(Axis::index_t x=0; x<xaxis.GetBinCountAll(); ++x )
                SetBinContent( x, y, z, 0 );
    entries = 0;
}

// ########################################################################
// ########################################################################

#ifdef TEST_HISTOGRAM3D

//#include "RootWriter.h"
//#include <TFile>

int main(int argc, char* argv[])
{
    Histogram3D h("ho", "hohoho", 10,0,10,"xho", 10,0,40, "yho", 10, 0, 20, "zho");
    h.Fill( 3,20, 7, 7);
    h.Fill( 4,19, 6, 9);
    h.Fill( 5,-2,1, 3 );
    h.Fill( -1,-1, 10, 4 );

    for(int iz=11; iz>=0; --iz) {
        for(int iy=11; iy>=0; --iy) {
            for(int ix=0; ix<12; ++ix)
                std::cout << h.GetBinContent(ix, iy, iz) << ' ';
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}

#endif