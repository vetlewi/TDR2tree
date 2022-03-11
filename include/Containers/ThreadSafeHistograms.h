//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#ifndef THREADSAFEHISTOGRAMS_H
#define THREADSAFEHISTOGRAMS_H

#include <Histograms.h>
#include <Histogram1D.h>
#include <Histogram2D.h>
#include <Histogram3D.h>

/*!
 * Thread safe histograms are histograms where the underlying memory for the histogram are stored thread safely.
 * Each thread will get an "adapter" class that buffers the entries in a vector. If the buffer is larger than
 * the min flush size the adapter class will try to lock a mutex, if failed it will continue filling the buffer
 * until the size is larger than the max flush size. Once this has been reached the adapter will wait until the
 * mutex is released and then flush its buffer.
 */

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <exception>
#include <stdexcept>

namespace ThreadSafeHistogramDetails {
    template<typename H>
    struct protected_object
    {
        std::mutex mutex;
        H object;
        protected_object(H _object) : mutex(), object(_object) {}
    };
}

template<typename T>
class ThreadSafeHistogram
{
private:
    std::mutex &mutex;
    T *histogram;

    const size_t min_buffer;
    const size_t max_buffer;
    typename T::buffer_t buffer;

    void flush()
    {
        for ( auto &element : buffer ){
            histogram->FillDirect(element);
        }
        buffer.clear();
    }

    void try_flush()
    {
        if ( mutex.try_lock() ){
            flush();
            mutex.unlock();
        }
    }

    void force_flush()
    {
        std::lock_guard lock(mutex);
        flush();
    }

    inline void check_buffer()
    {
        if ( buffer.size() < min_buffer )
            return;
        else if ( buffer.size() < max_buffer )
            try_flush();
        else
            force_flush();
    }

public:

    ThreadSafeHistogram(std::mutex &_mutex, T *_histogram,
                        const size_t &_min_buffer = 1024, const size_t &_max_buffer = 16384)
        : mutex( _mutex )
        , histogram( _histogram )
        , min_buffer( _min_buffer )
        , max_buffer( _max_buffer )
    {
        buffer.reserve( max_buffer );
    }

    ~ThreadSafeHistogram()
    {
        force_flush();
    }

    /*template<class... Args>
    void Fill(Args... args){
        buffer.emplace_back(std::forward<Args>(args)...);
        if ( buffer.size() < min_buffer )
            return;
        else if ( buffer.size() < max_buffer )
            try_flush();
        else
            force_flush();
    }*/

    void Fill(const Axis::bin_t &x)
    {
        buffer.push_back({x, 1});
        check_buffer();
    }

    void Fill(const Axis::bin_t &x, const Axis::bin_t &y)
    {
        buffer.push_back({x, y, 1});
        check_buffer();
    }

    void Fill(const Axis::bin_t &x, const Axis::bin_t &y, const Axis::bin_t &z)
    {
        buffer.push_back({x, y, z, 1});
        check_buffer();
    }

};

typedef ThreadSafeHistogram<Histogram1D> ThreadSafeHistogram1D;
typedef ThreadSafeHistogram<Histogram2D> ThreadSafeHistogram2D;
typedef ThreadSafeHistogram<Histogram3D> ThreadSafeHistogram3D;

class ThreadSafeHistograms
{
private:
    Histograms histograms;

    const size_t min_buffer;
    const size_t max_buffer;

    typedef ThreadSafeHistogramDetails::protected_object<Histogram1Dp>* p1d;
    typedef ThreadSafeHistogramDetails::protected_object<Histogram2Dp>* p2d;
    typedef ThreadSafeHistogramDetails::protected_object<Histogram3Dp>* p3d;

    std::map<std::string, p1d> map1d;
    std::map<std::string, p2d> map2d;
    std::map<std::string, p3d> map3d;


    template<typename T>
    static typename T::value_type::second_type Get(T map, const std::string &name)
    {
        const auto obj = map.find(name);
        if ( obj != map.end() ){
            return obj->second;
        } else {
            throw std::out_of_range("Not defined");
        }
    }

    ThreadSafeHistogram1D Get1D(const std::string &name)
    {
        auto p = Get(map1d, name);
        return {p->mutex, p->object, min_buffer, max_buffer};
    }

    ThreadSafeHistogram2D Get2D(const std::string &name)
    {
        auto p = Get(map2d, name);
        return {p->mutex, p->object, min_buffer, max_buffer};
    }

    ThreadSafeHistogram3D Get3D(const std::string &name)
    {
        auto p = Get(map3d, name);
        return {p->mutex, p->object, min_buffer, max_buffer};
    }

public:

    ThreadSafeHistograms(const size_t &min_buf = 1024, const size_t &max_buf = 16384)
        : min_buffer( min_buf ), max_buffer( max_buf ){}

    ~ThreadSafeHistograms()
    {
        for ( auto hist : map1d ){
            delete hist.second;
        }
        for ( auto hist : map2d ){
            delete hist.second;
        }
        for ( auto hist : map3d ){
            delete hist.second;
        }
    }

    ThreadSafeHistogram1D Create1D( const std::string& name,  /*!< The name of the new histogram. */
                                    const std::string& title, /*!< The title of teh new histogram. */
                                    Axis::index_t channels,   /*!< The number of regular bins. */
                                    Axis::bin_t left,         /*!< The lower edge of the lowest bin.  */
                                    Axis::bin_t right,        /*!< The upper edge of the highest bin. */
                                    const std::string& xtitle /*!< The title of the x axis. */)
    {
        try {
            return Get1D(name);
        } catch ( std::out_of_range &e ){
            // The histogram doesn't exist, we will create it now.
            p1d hist = new ThreadSafeHistogramDetails::protected_object<Histogram1Dp>(histograms.Create1D(name, title, channels, left, right, xtitle));
            map1d[name] = hist;
            return {hist->mutex, hist->object};
        }
    }


    ThreadSafeHistogram2D Create2D( const std::string& name,   /*!< The name of the new histogram. */
                                    const std::string& title,  /*!< The title of teh new histogram. */
                                    Axis::index_t xchannels,   /*!< The number of regular bins on the x axis. */
                                    Axis::bin_t xleft,         /*!< The lower edge of the lowest bin on the x axis. */
                                    Axis::bin_t xright,        /*!< The upper edge of the highest bin on the x axis. */
                                    const std::string& xtitle, /*!< The title of the x axis. */
                                    Axis::index_t ychannels,   /*!< The number of regular bins on the y axis. */
                                    Axis::bin_t yleft,         /*!< The lower edge of the lowest bin on the y axis. */
                                    Axis::bin_t yright,        /*!< The upper edge of the highest bin on the y axis. */
                                    const std::string& ytitle  /*!< The title of the y axis. */)
    {
        try {
            return Get2D(name);
        } catch ( std::out_of_range &e ){
            // The histogram doesn't exist, we will create it now.
            p2d hist =
                    new ThreadSafeHistogramDetails::protected_object<Histogram2Dp>(
                            histograms.Create2D(name, title,
                                                       xchannels, xleft, xright, xtitle,
                                                       ychannels, yleft, yright, ytitle));
            map2d[name] = hist;
            return {hist->mutex, hist->object};
        }
    }

    ThreadSafeHistogram3D Create3D( const std::string& name,   /*!< The name of the new histogram. */
                                    const std::string& title,  /*!< The title of teh new histogram. */
                                    Axis::index_t xchannels,   /*!< The number of regular bins on the x axis. */
                                    Axis::bin_t xleft,         /*!< The lower edge of the lowest bin on the x axis. */
                                    Axis::bin_t xright,        /*!< The upper edge of the highest bin on the x axis. */
                                    const std::string& xtitle, /*!< The title of the x axis. */
                                    Axis::index_t ychannels,   /*!< The number of regular bins on the y axis. */
                                    Axis::bin_t yleft,         /*!< The lower edge of the lowest bin on the y axis. */
                                    Axis::bin_t yright,        /*!< The upper edge of the highest bin on the y axis. */
                                    const std::string& ytitle, /*!< The title of the y axis. */
                                    Axis::index_t zchannels,   /*!< The number of regular bins on the z axis. */
                                    Axis::bin_t zleft,         /*!< The lower edge of the lowest bin on the z axis. */
                                    Axis::bin_t zright,        /*!< The upper edge of the highest bin on the z axis. */
                                    const std::string& ztitle  /*!< The title of the z axis. */)
    {
        try {
            return Get3D(name);
        } catch (std::out_of_range &e) {
            // The histogram doesn't exist, we will create it now.
            p3d hist = new ThreadSafeHistogramDetails::protected_object<Histogram3Dp>(
                    histograms.Create3D(name, title,
                                        xchannels, xleft, xright, xtitle,
                                        ychannels, yleft, yright, ytitle,
                                        zchannels, zleft, zright, ztitle));
            map3d[name] = hist;
            return {hist->mutex, hist->object};
        }
    }

    Histograms &GetHistograms(){ return histograms; }

};


#endif // THREADSAFEHISTOGRAMS_H
