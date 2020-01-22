#include "Modules/Essentials/Essentials.hpp"
#include "Modules/Essentials/FileManager.hpp"

#include "Image.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>

#include <CGAL/Triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>

#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_3.h>

#include <fstream>
#include <vector>
#include <math.h>

typedef std::vector<int> Histogram;

std::vector<uint> saveHistogram(std::string folder, std::string name, Histogram edge){
    std::vector<uint> histogram;
    std::string fullpath = folder + name;
    std::ofstream output;
    output.open( fullpath.c_str() );
    
    int limit;
    if(edge.size() > 181 ){ limit = 80;
    } else { limit = 70; }
    
    for(auto i = 0 ; i <= limit ; ++i){
        output << i << "\t" << edge[i] << "\n";
        histogram.push_back(edge[i]);
    }
    output.close();
    return histogram;
}


std::vector<uint> saveHistogram(std::string folder, std::string name, Histogram edge, Histogram interior ){
    std::vector<uint> histogram;
    std::string fullpath = folder + name;
    std::ofstream output;
    output.open( fullpath.c_str() );
    
    int limit;
    if(edge.size() > 181 ){ limit = 80;
    } else { limit = 70; }
    
    output << "a,b,c\n";
    for(auto i = 0 ; i <= limit ; ++i){
        output << i << "," << edge[i] << "," << interior[i] << "\n";
        histogram.push_back(edge[i]);
    }
    output.close();
    return histogram;
}

class CGALTriangulation{
protected:
    brahand::ImageSize size;
    brahand::IndicesArray generators;
    std::ofstream file;
    
public:
    brahand::uint elementsCount; // triangles and tetraedrals
    
    CGALTriangulation(brahand::IndicesArray generators, brahand::ImageSize size){
        this->generators = generators;
        this->size = size;
    }
    
    virtual Histogram generateHistogram(std::string folder, std::string name, brahand::ImageWrapper<float> image) = 0;
    virtual void triangulate() = 0;
    virtual void show_vertices(brahand::Image image) = 0;
};

class CGALTriangulation2D : public CGALTriangulation {
private:
    typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

    typedef CGAL::Triangulation_vertex_base_with_info_2<int, Kernel> Vb;
    typedef CGAL::Triangulation_face_base_with_info_2<int, Kernel> Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;

    typedef CGAL::Delaunay_triangulation_2<Kernel, Tds> Delaunay2;
    
    typedef Kernel::Point_2 Point2;
    typedef Kernel::Vector_2 Vector2;
    typedef Delaunay2::Finite_faces_iterator Finite_faces_iterator;
    
    Delaunay2 triangulation;
    
    inline double angle ( Delaunay2::Vertex_handle a, Delaunay2::Vertex_handle b, Delaunay2::Vertex_handle c ) const{
        Vector2 x = a->point() - b->point();
        Vector2 y = c->point() - b->point();

        return acos( ( x*y ) / ( sqrt( x*x ) * sqrt( y*y ) ) ) * 180.0 / M_PI;
    }

    inline double face_angle(Delaunay2::Face_handle f) const {
        double a1 = angle( f->vertex(0), f->vertex(1), f->vertex(2) ); if(isnan(a1)) a1 = 180.0;
        double a2 = angle( f->vertex(1), f->vertex(2), f->vertex(0) ); if(isnan(a2)) a2 = 180.0;
        double a3 = 180 - a1 - a2; if(isnan(a3)) a3 = 180.0;
        return std::min(a1, std::min(a2, a3));
    }
    
    inline Histogram anglesHistogram(Delaunay2 & dt) const {
        Histogram histogram(181, 0);
        for(Finite_faces_iterator it = dt.finite_faces_begin(); it != dt.finite_faces_end(); it++){
            auto minAngle = (int) face_angle( it );
            histogram[minAngle]++;
        }
        return histogram;
    }
    
    void colorPattern(Delaunay2 triangulation, std::string folder, std::string name, brahand::ImageWrapper<float> image){
        
        std::ofstream os;
        os.open(folder + name + ".vtk");
        
        std::map<int,std::map<int,int>> vertexIndex;
        
        os << "# vtk DataFile Version 1.0\n";
        os << "Unstructured Grid Example\n";
        os << "ASCII\n\n";
        
        os << "DATASET UNSTRUCTURED_GRID\n";
        
        os << "POINTS " << triangulation.number_of_vertices() << " float\n";
        int idx = 0;
        Delaunay2::Finite_vertices_iterator verticesIterator;
        for(verticesIterator = triangulation.finite_vertices_begin(); verticesIterator != triangulation.finite_vertices_end() ; ++verticesIterator){
            os << verticesIterator->point() << " 0\n";
            vertexIndex[verticesIterator->point().x()][verticesIterator->point().y()] = idx;
            idx++;
        }
        
        os << "\nCELLS " << triangulation.number_of_faces() << " " << triangulation.number_of_faces()*4 << "\n";
        Delaunay2::Finite_faces_iterator facesIterator;
        for(facesIterator = triangulation.finite_faces_begin() ; facesIterator != triangulation.finite_faces_end() ; ++facesIterator){
            Delaunay2::Face_handle face = facesIterator;
            os << "3 " <<   vertexIndex[face->vertex(0)->point().x()][face->vertex(0)->point().y()] << " ";
            os << vertexIndex[face->vertex(1)->point().x()][face->vertex(1)->point().y()] << " ";
            os << vertexIndex[face->vertex(2)->point().x()][face->vertex(2)->point().y()] << "\n";
        }
        
        os << "\nCELL_TYPES " << triangulation.number_of_faces() << "\n";
        for(size_t i = 0 ;i < triangulation.number_of_faces() ; ++i) { os << "5 ";}
        
        // os << "\n\nCELL_DATA " << triangulation.number_of_faces() << "\n";
        // os << "COLOR_SCALARS ColorSpace 1\n";

        // for(auto it = triangulation.finite_faces_begin() ; it != triangulation.finite_faces_end() ; ++it){
        //     auto v1 = it->vertex(0)->point();
        //     auto v2 = it->vertex(1)->point();
        //     auto v3 = it->vertex(2)->point();
            
        //     auto c1 = image.imagePointer[((image.size.height - v1.y()) * image.size.width) + v1.x()];
        //     auto c2 = image.imagePointer[((image.size.height - v2.y()) * image.size.width) + v2.x()];
        //     auto c3 = image.imagePointer[((image.size.height - v3.y()) * image.size.width) + v3.x()];
        //     os << (float(c1 + c2 + c3) / 3.0) / 255.0 << "\n";
        // }

        os << "\n\nPOINT_DATA " << triangulation.number_of_vertices() << "\n";
        os << "FIELD FieldData 1\n";
        os << "VERTEX_DENSITY " << "1 " << triangulation.number_of_vertices() << " float\n";
        for(verticesIterator = triangulation.finite_vertices_begin(); verticesIterator != triangulation.finite_vertices_end() ; ++verticesIterator){
            os << image.imagePointer[((image.size.height - verticesIterator->point().y() - 1) * image.size.width) + verticesIterator->point().x()] << "\n";
        }

        os << "\n\nCELL_DATA " << triangulation.number_of_faces() << "\n";
        os << "FIELD FieldData 1\n";
        os << "CELL_NEIGHBORS " << triangulation.dimension() + 1 << " " << triangulation.number_of_faces() << " int\n";
        for(facesIterator = triangulation.finite_faces_begin() ; facesIterator != triangulation.finite_faces_end() ; ++facesIterator){
            Delaunay2::Face_handle face = facesIterator;
            auto neighbor_0 = triangulation.is_infinite(face->neighbor(0)) ? -1 : face->neighbor(0)->info();
            auto neighbor_1 = triangulation.is_infinite(face->neighbor(1)) ? -1 : face->neighbor(1)->info();
            auto neighbor_2 = triangulation.is_infinite(face->neighbor(2)) ? -1 : face->neighbor(2)->info();
            os << neighbor_0 << " " << neighbor_1 << " " << neighbor_2 << "\n";
            // if(triangulation.is_infinite(neighbor_0)) {
            //     std::cout << -1 << std::endl;
            // } else {
            //     std::cout << &(*triangulation.finite_faces_begin()) << std::endl;
            // }
                
            // os << face->neighbor(0)->index() << " " << face->neighbor(1)->index() << face->neighbor(2)->index() << "\n";
            // os << vertexIndex[face->vertex(0)->point().x()][face->vertex(0)->point().y()] << " ";
            // os << vertexIndex[face->vertex(1)->point().x()][face->vertex(1)->point().y()] << " ";
            // os << vertexIndex[face->vertex(2)->point().x()][face->vertex(2)->point().y()] << "\n";
        }

        
        os.close();
    }

    
public:
    CGALTriangulation2D(brahand::IndicesArray generators, brahand::ImageSize size) : CGALTriangulation(generators, size){}
    
    void triangulate(){
        std::vector<Point2> points;
        brahand::uint idx;
        short layer, row, col;
        
        for( brahand::uint i = 0 ; i < this->generators.count ; ++i ) {
            idx = this->generators[i];
            layer = idx / (this->size.width * this->size.height);
            idx -= (layer * this->size.width * this->size.height);
            row = idx / this->size.width;
            col = idx % this->size.width;
            
            points.push_back(Point2(col,(size.height - 1) -row)); /// (height - row) to draw with vtk
        }
        
        Delaunay2 delaunay(points.begin(), points.end());

        int vid = 0;
        Delaunay2::Finite_vertices_iterator verticesIterator;
        for(vid = 0, verticesIterator = delaunay.finite_vertices_begin(); verticesIterator != delaunay.finite_vertices_end() ; ++verticesIterator){
            Delaunay2::Vertex_handle vertex = verticesIterator;
            vertex->info() = vid++;
        }
        
        int cid = 0;
        Delaunay2::Finite_faces_iterator facesIterator;
        for(cid = 0, facesIterator = delaunay.finite_faces_begin() ; facesIterator != delaunay.finite_faces_end() ; ++facesIterator){
            Delaunay2::Face_handle face = facesIterator;
            face->info() = cid++;
            // if(cid < 10) std::cout << cid << " : " << face->vertex(0)->info() << " " << face->vertex(1)->info() << " " << face->vertex(2)->info() << std::endl;
        }

        this->elementsCount = delaunay.number_of_faces();
        this->triangulation = delaunay;
    }
    
    Histogram generateHistogram(std::string folder, std::string name, brahand::ImageWrapper<float> image){
        Histogram histogram = anglesHistogram(this->triangulation);
        colorPattern(this->triangulation, folder, name, image);

        return histogram;
    }

    void show_vertices(brahand::Image image){
        unsigned int uh = image.size.height;
        unsigned int uw = image.size.width;
        for(auto fit = this->triangulation.finite_faces_begin(); fit != this->triangulation.finite_faces_end() ; ++fit){
        //    std::cout << fit->vertex(0) << " " << verticesIterator->point().y() << std::endl;
           auto v0 = fit->vertex(0)->point();
           auto v1 = fit->vertex(1)->point();
           auto v2 = fit->vertex(2)->point();

           auto centroid = Point2((v0.x() + v1.x() + v2.x())/3, (v0.y() + v1.y() + v2.y())/3);
           unsigned int l = ceilf(centroid.x());
           unsigned int k = ceilf(centroid.y());
           float a = centroid.x() - l;
           float b = centroid.y() - k;

            auto lk00 = (uh - k) * uw + l;
            auto lk10 = (uh - k) * uw + (l+1);
            auto lk01 = (uh - (k+1)) * uw + l;
            auto lk11 = (uh - (k+1)) * uw + (l+1);

            auto ccolor = ((1-a)*(1-b))*image.imagePointer[lk00] + ((a)*(1-b))*image.imagePointer[lk01] + ((1-a)*(b))*image.imagePointer[lk10] + ((a)*(b))*image.imagePointer[lk11];

        //    std::cout << centroid.x() << " " << centroid.y() << " | " << l << " " << k << " | " << a << " " << b << " | " << uh << " " << uw << " | " << 
        //     lk00 << " (" << image.imagePointer[lk00] << ") " <<
        //     lk10 << " (" << image.imagePointer[lk01] << ") " <<
        //     lk01 << " (" << image.imagePointer[lk10] << ") " <<
        //     lk11 << " (" << image.imagePointer[lk11] << ") " <<
        //     " -> " << ccolor <<
        //     std::endl;
        }

        std::vector<std::pair<unsigned int, unsigned int>> edges;
        for(auto fit = this->triangulation.finite_faces_begin(); fit != this->triangulation.finite_faces_end() ; ++fit){
            
        }

   }

};


class CGALTriangulation3D : public CGALTriangulation {
private:
    typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

    typedef CGAL::Triangulation_vertex_base_with_info_3<int, Kernel> Vb;
    typedef CGAL::Triangulation_cell_base_with_info_3<int, Kernel> Cb;
    typedef CGAL::Triangulation_data_structure_3<Vb,Cb> Tds;

    typedef CGAL::Delaunay_triangulation_3<Kernel, Tds> Delaunay3;

    typedef Kernel::Point_3 Point3;
    typedef Kernel::Vector_3 Vector3;
    typedef Delaunay3::Finite_cells_iterator Finite_cells_iterator;

    Delaunay3 triangulation;

   
   double dihedral_angle(Point3 a, Point3 b, Point3 c, Point3 d) const {
       Vector3 b1 = c - a ;
       Vector3 b2 = b - a ;
       Vector3 b3 = d - a ;
       Vector3 c1 = CGAL::cross_product(b1,b2);
       Vector3 c2 = CGAL::cross_product(b2,b3);
       return acos(- ( c1*( 1/ sqrt(c1*c1) ) * c2*( 1/ sqrt(c2*c2) )  ))* 180.0 / M_PI;
   }
   
   double cell_angle(Delaunay3::Cell_handle it) const {
       std::vector<double> dangles(6);
       Point3 v0 = it->vertex(0)->point();
       Point3 v1 = it->vertex(1)->point();
       Point3 v2 = it->vertex(2)->point();
       Point3 v3 = it->vertex(3)->point();
       dangles[0] = dihedral_angle(v0,v1,v2,v3); if(isnan(dangles[0])) dangles[0] = 360.0;
       dangles[1] = dihedral_angle(v1,v2,v0,v3); if(isnan(dangles[1])) dangles[0] = 360.0;
       dangles[2] = dihedral_angle(v0,v2,v1,v3); if(isnan(dangles[2])) dangles[0] = 360.0;
       dangles[3] = dihedral_angle(v0,v3,v2,v1); if(isnan(dangles[3])) dangles[0] = 360.0;
       dangles[4] = dihedral_angle(v3,v1,v2,v0); if(isnan(dangles[4])) dangles[0] = 360.0;
       dangles[5] = dihedral_angle(v2,v3,v0,v1); if(isnan(dangles[5])) dangles[0] = 360.0;
       
       auto min1 = std::min(dangles[0], std::min(dangles[1], std::min(dangles[2], dangles[3])));
       return std::min(min1, std::min(dangles[4], dangles[5]));
   }
   
   inline Histogram anglesHistogram(Delaunay3 & dt) const {
       Histogram histogram(361, 0);
       for(Finite_cells_iterator it = dt.finite_cells_begin(); it != dt.finite_cells_end(); it++){
           auto minAngle = (int) cell_angle( it );
           histogram[minAngle]++;
       }
       return histogram;
       
   }
   
   void colorPattern3D(Delaunay3 triangulation, std::string folder, std::string name, brahand::ImageWrapper<float> image){
       
       std::ofstream os;
       os.open(folder + name + ".vtk");
       

       std::map<int,std::map<int,std::map<int,uint>>> vertexIndex;
       
       os << "# vtk DataFile Version 1.0\n";
       os << "Unstructured Grid Example\n";
       os << "ASCII\n\n";
       
       os << "DATASET UNSTRUCTURED_GRID\n";
       
       os << "POINTS " << triangulation.number_of_vertices() << " float\n";
       
       Delaunay3::Finite_vertices_iterator verticesIterator;
       for(verticesIterator = triangulation.finite_vertices_begin(); verticesIterator != triangulation.finite_vertices_end() ; ++verticesIterator){
           os << verticesIterator->point() << "\n";
       }

       os << "\nCELLS " << triangulation.number_of_finite_cells() << " " << triangulation.number_of_finite_cells()*5 << "\n";
       
       for(Finite_cells_iterator it = triangulation.finite_cells_begin(); it != triangulation.finite_cells_end(); it++){
           Delaunay3::Cell_handle cell = it;
           os << "4 " << cell->vertex(0)->info() << " " << cell->vertex(1)->info() << " " << cell->vertex(2)->info() << " " << cell->vertex(3)->info() << "\n";
       }

       os << "\nCELL_TYPES " << triangulation.number_of_finite_cells() << "\n";
       int cells = triangulation.number_of_finite_cells();
       for(int i = 0 ;i < cells ; ++i) { os << "10 "; }

        os << "\n\nPOINT_DATA " << triangulation.number_of_vertices() << "\n";
        os << "FIELD FieldData 1\n";
        os << "VERTEX_DENSITY " << "1 " << triangulation.number_of_vertices() << " float\n";
        for(verticesIterator = triangulation.finite_vertices_begin(); verticesIterator != triangulation.finite_vertices_end() ; ++verticesIterator){
            os << image.imagePointer[(verticesIterator->point().z()*image.size.width + (image.size.height - verticesIterator->point().y() - 1) * image.size.width) + verticesIterator->point().x()] << "\n";
        }

        os << "\n\nCELL_DATA " << triangulation.number_of_finite_cells() << "\n";
        os << "FIELD FieldData 1\n";
        os << "CELL_NEIGHBORS " << triangulation.dimension() + 1 << " " << triangulation.number_of_finite_cells() << " int\n";
        for(Finite_cells_iterator cellsIterator = triangulation.finite_cells_begin() ; cellsIterator != triangulation.finite_cells_end() ; ++cellsIterator){
            typename Delaunay3::Cell_handle cell = cellsIterator;
            auto neighbor_0 = triangulation.is_infinite(cell->neighbor(0)) ? -1 : cell->neighbor(0)->info();
            auto neighbor_1 = triangulation.is_infinite(cell->neighbor(1)) ? -1 : cell->neighbor(1)->info();
            auto neighbor_2 = triangulation.is_infinite(cell->neighbor(2)) ? -1 : cell->neighbor(2)->info();
            auto neighbor_3 = triangulation.is_infinite(cell->neighbor(3)) ? -1 : cell->neighbor(3)->info();
            os << neighbor_0 << " " << neighbor_1 << " " << neighbor_2 << " " << neighbor_3 << "\n";
        }
       
       os.close();
   }
   
public:
   
   CGALTriangulation3D(brahand::IndicesArray generators, brahand::ImageSize size) : CGALTriangulation(generators, size){}
   
   void triangulate(){
       std::vector<Point3> points;
       brahand::uint idx;
       short layer, row, col;
       
       for( brahand::uint i = 0 ; i < this->generators.count ; ++i ) {
           idx = this->generators[i];
           layer = idx / (this->size.width * this->size.height);
           idx -= (layer * this->size.width * this->size.height);
           row = idx / this->size.width;
           col = idx % this->size.width;
           
           points.push_back(Point3(col,row,layer));
       }
       
       Delaunay3 delaunay(points.begin(), points.end());

        int vid = 0;
        Delaunay3::Finite_vertices_iterator verticesIterator;
        for(vid = 0, verticesIterator = delaunay.finite_vertices_begin(); verticesIterator != delaunay.finite_vertices_end() ; ++verticesIterator){
            Delaunay3::Vertex_handle vertex = verticesIterator;
            vertex->info() = vid++;
        }
        
        int cid = 0;
        Delaunay3::Finite_cells_iterator cellsIterator;
        for(cid = 0, cellsIterator = delaunay.finite_cells_begin() ; cellsIterator != delaunay.finite_cells_end() ; ++cellsIterator){
            typename Delaunay3::Cell_handle cell = cellsIterator;
            cell->info() = cid++;
            // if(cid < 10) std::cout << cid << " : " << face->vertex(0)->info() << " " << face->vertex(1)->info() << " " << face->vertex(2)->info() << std::endl;
        }

       this->elementsCount = delaunay.number_of_finite_cells();
       this->triangulation = delaunay;
   }
   
   Histogram generateHistogram(std::string folder, std::string name, brahand::ImageWrapper<float> image){
       Histogram histogram = anglesHistogram(this->triangulation);
       colorPattern3D(this->triangulation, folder, name, image);
       return histogram;
   }

   void show_vertices(brahand::Image image){}

};

