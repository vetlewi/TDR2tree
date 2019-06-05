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

class TCutG;

class PolygonGate {

private:
    TCutG *cut;
    CGAL::Polygon_2<CGAL::Exact_predicates_inexact_constructions_kernel> polygon;
    bool set;
public:
    PolygonGate() : set( false ){}

    PolygonGate(const Points_t *points, const int &size);

    explicit PolygonGate(const char *fname);

    //~PolygonGate(){ if ( cut ) delete cut; }

    void Set(const char *fname);

    bool operator()(const Points_t &point) const;

    inline bool operator()(const double &x, const double &y) const { return operator()({x,y}); }

    inline bool IsSet() const { return set; }
};


#endif // POLYGONGATE_H
