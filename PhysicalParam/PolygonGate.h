//
// Created by Vetle Wegner Ingeberg on 2019-05-31.
//

#ifndef POLYGONGATE_H
#define POLYGONGATE_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>

struct Points_t {
    double x;
    double y;
};

class PolygonGate {

private:
    CGAL::Polygon_2<CGAL::Exact_predicates_inexact_constructions_kernel> polygon;

public:
    PolygonGate(const Points_t *points, const int &size);

    bool operator()(const Points_t &point) const;

};


#endif // POLYGONGATE_H
