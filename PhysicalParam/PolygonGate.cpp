//
// Created by Vetle Wegner Ingeberg on 2019-05-31.
//

#include "PolygonGate.h"

#include <iostream>

PolygonGate::PolygonGate(const Points_t *points, const int &size)
{
    for (int i = 0 ; i < size ; ++i){
        polygon.push_back(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(points[i].x, points[i].y));
    }

    std::cout << "The polygon is ";
    std::cout << ( polygon.is_simple() ) ? "" : "not ";
    std::cout << "simple." << std::endl;
}

bool PolygonGate::operator()(const Points_t &point) const {
    switch ( polygon.bounded_side(CGAL::Exact_predicates_inexact_constructions_kernel::Point_2(point.x, point.y)) ){
        case CGAL::ON_UNBOUNDED_SIDE : {
            return false;
        }
        case CGAL::ON_BOUNDARY : {
            return true;
        }
        case CGAL::ON_BOUNDED_SIDE : {
            return true;
        }
    }
}
