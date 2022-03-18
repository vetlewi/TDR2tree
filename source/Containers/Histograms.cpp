
#include "Histograms.h"

#include "Histogram1D.h"
#include "Histogram2D.h"
#include "Histogram3D.h"

#include <iostream>

Named::Named( const std::string& nm, const std::string& ttl, const std::string& pth)
    : name( nm )
    , title( ttl )
    , path( pth )
{
}

// ########################################################################
// ########################################################################

Axis::Axis(const std::string& name, index_t c, bin_t l, bin_t r, const std::string& t )
    : Named( name, t )
    , channels2( c+2 )
    , left( l )
    , right( r )
{
  const double cbw = (right-left)/double(channels2-2);
  binwidth = (bin_t)cbw;
  /*if( cbw != double(binwidth) )
    std::cout << "non-int binwidth for axis '" << name << "'" << std::endl;
  if( binwidth == 0 )
    std::cout << "zero binwidth for axis '" << name << "'" << std::endl;
  if ( binwidth < 0 )
    std::cout << "negative binwidth for axis '" << name << "'" << std::endl;
    */
}

// ########################################################################
// ########################################################################

Histograms::~Histograms()
{
  for(auto & it : map1d)
      delete it.second;
  for(auto & it : map2d)
      delete it.second;
  for(auto & it : map3d)
      delete it.second;
}

// ########################################################################

Histogram1Dp Histograms::Create1D( const std::string& name, const std::string& title,
                                   Axis::index_t c, Axis::bin_t l, Axis::bin_t r, const std::string& xtitle,
                                   const std::string& path)
{
  Histogram1Dp h(new Histogram1D(name, title, c, l, r, xtitle, path));
  map1d[ name ] = h;
  return h;
}

// ########################################################################

Histogram2Dp Histograms::Create2D( const std::string& name, const std::string& title,
                                   Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xtitle,
                                   Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& ytitle,
                                   const std::string& path)
{
  Histogram2Dp h(new Histogram2D(name, title, ch1, l1, r1, xtitle, ch2, l2, r2, ytitle, path));
  map2d[ name ] = h;
  return h;
}

// ########################################################################

Histogram3Dp Histograms::Create3D( const std::string& name, const std::string& title,
                                   Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xtitle,
                                   Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& ytitle,
                                   Axis::index_t ch3, Axis::bin_t l3, Axis::bin_t r3, const std::string& ztitle,
                                   const std::string& path)
{
    Histogram3Dp h(new Histogram3D(name, title, ch1, l1, r1, xtitle, ch2, l2, r2, ytitle, ch3, l3, r3, ztitle, path));
    map3d[ name ] = h;
    return h;
}

// ########################################################################

void Histograms::ResetAll()
{
  for(auto & it : map1d)
    it.second->Reset();
  for(auto & it : map2d)
    it.second->Reset();
  for(auto & it : map3d)
    it.second->Reset();
}

// ########################################################################

Histogram1Dp Histograms::Find1D( const std::string& name )
{
  auto it = map1d.find( name );
  if( it != map1d.end() )
    return it->second;
  else
    return nullptr;
}

// ########################################################################

Histogram2Dp Histograms::Find2D( const std::string& name )
{
  auto it = map2d.find( name );
  if( it != map2d.end() )
    return it->second;
  else
    return nullptr;
}

// ########################################################################

Histogram3Dp Histograms::Find3D( const std::string& name )
{
    auto it = map3d.find( name );
    if( it != map3d.end() )
        return it->second;
    else
        return nullptr;
}

// ########################################################################

void Histograms::Merge(Histograms& other)
{
  for(auto & it : map1d) {
    Histogram1Dp me = it.second;
    Histogram1Dp you = other.Find1D( me->GetName() );
    if( you )
      me->Add( you, 1 );
  }
  for(auto & it : map2d) {
    Histogram2Dp me = it.second;
    Histogram2Dp you = other.Find2D( me->GetName() );
    if( you )
      me->Add( you, 1 );
  }
    for(auto & it : map3d) {
        Histogram3Dp me = it.second;
        Histogram3Dp you = other.Find3D( me->GetName() );
        if( you )
            me->Add( you, 1 );
    }
}

// ########################################################################

Histograms::list1d_t Histograms::GetAll1D()
{
  list1d_t list1d;
  for(auto & it : map1d)
    list1d.push_back( it.second );
  return list1d;
}

// ########################################################################

Histograms::list2d_t Histograms::GetAll2D()
{
  list2d_t list2d;
  for(auto & it : map2d)
    list2d.push_back( it.second );
  return list2d;
}

// ########################################################################

Histograms::list3d_t Histograms::GetAll3D()
{
    list3d_t list3d;
    for(auto & it : map3d)
        list3d.push_back( it.second );
    return list3d;
}

// ########################################################################