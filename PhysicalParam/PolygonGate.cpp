//
// Created by Vetle Wegner Ingeberg on 2019-05-31.
//

#include "PolygonGate.h"

#include <iostream>
#include <fstream>

#include <vector>
#include <utility>

#include <TCutG.h>

PolygonGate::PolygonGate(const Points_t *points, const int &size)
    : set( true )
{
    for (int i = 0 ; i < size ; ++i){
        polygon.push_back(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(points[i].x, points[i].y));
    }

    std::cout << "The polygon is ";
    std::cout << ( polygon.is_simple() ? "" : "not ");
    std::cout << "simple." << std::endl;
}

PolygonGate::PolygonGate(const char *fname)
    : set( true )
{
    std::ifstream file(fname);
    if ( !file.is_open() ){
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << fname << std::endl;
    }
    double x, y;
    std::vector<std::pair<double,double> > points;
    while ( file ){
        file >> x >> y;
        points.emplace_back(x, y);
        polygon.push_back(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(x, y));
    }

    cut = new TCutG("cut", points.size());
    int setting = 0;
    for ( auto point : points ){
        cut->SetPoint(setting, point.first, point.second);
    }

    std::cout << "The polygon is ";
    std::cout << ( polygon.is_simple() ? "" : "not ");
    std::cout << "simple." << std::endl;
}

void PolygonGate::Set(const char *fname)
{
    if ( set ){
        std::cerr << __PRETTY_FUNCTION__ << ": Is already set!" << std::endl;
        return;
    }
    std::ifstream file(fname);
    if ( !file.is_open() ){
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << fname << std::endl;
    }
    double x, y;
    std::vector<std::pair<double,double> > points;
    while ( file ){
        file >> x >> y;
        points.emplace_back(x, y);
        polygon.push_back(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(x, y));
    }

    if ( cut )
        delete cut;

    cut = new TCutG("cut", points.size());
    int setting = 0;
    for ( auto point : points ){
        cut->SetPoint(setting, point.first, point.second);
    }

    std::cout << "The polygon is ";
    std::cout << ( polygon.is_simple() ? "" : "not ");
    std::cout << "simple." << std::endl;
    set = true;
}

bool PolygonGate::operator()(const Points_t &point) const {
    return cut->IsInside(point.x, point.y);
    /*switch ( polygon.bounded_side(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(point.x, point.y)) ){
        case CGAL::ON_UNBOUNDED_SIDE : {
            return false;
        }
        case CGAL::ON_BOUNDARY : {
            return true;
        }
        case CGAL::ON_BOUNDED_SIDE : {
            return true;
        }
    }*/
}
