#ifndef CGAL_SCENE_SURFACE_MESH_ITEM_H
#define CGAL_SCENE_SURFACE_MESH_ITEM_H
//Defines the precision of the positions (for performance/precision sake)
#define CGAL_GL_DATA GL_FLOAT
#define cgal_gl_data float
#define CGAL_IS_FLOAT 1

#include "Scene_surface_mesh_item_config.h"
#include <CGAL/Three/Scene_item.h>
#include <CGAL/Three/Viewer_interface.h>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>

#include <CGAL/Surface_mesh/Surface_mesh_fwd.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <QColor>

#include "properties.h"

struct Scene_surface_mesh_item_priv;

class SCENE_SURFACE_MESH_ITEM_EXPORT Scene_surface_mesh_item
  : public CGAL::Three::Scene_item
{
  Q_OBJECT
public:
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
  typedef Kernel::Point_3 Point;
  typedef CGAL::Surface_mesh<Point> SMesh;
  typedef SMesh FaceGraph;
  typedef boost::graph_traits<SMesh>::face_descriptor face_descriptor;
  typedef boost::graph_traits<SMesh>::vertex_descriptor vertex_descriptor;
  typedef boost::graph_traits<SMesh>::halfedge_descriptor halfedge_descriptor;
  typedef SMesh::Property_map<vertex_descriptor,int> Vertex_selection_map;
  typedef SMesh::Property_map<face_descriptor,int> Face_selection_map;
  typedef SMesh::Property_map<halfedge_descriptor, bool> Halfedge_is_feature_map;
  typedef SMesh::Property_map<face_descriptor, int> Face_patch_id_map;
  typedef SMesh::Property_map<vertex_descriptor,int> Vertex_num_feature_edges_map;


  Scene_surface_mesh_item();
  // Takes ownership of the argument.
  Scene_surface_mesh_item(SMesh*);
  Scene_surface_mesh_item(SMesh);
  Scene_surface_mesh_item(const Scene_surface_mesh_item& other);

  ~Scene_surface_mesh_item();


  Scene_surface_mesh_item* clone() const Q_DECL_OVERRIDE;
  void draw(CGAL::Three::Viewer_interface *) const Q_DECL_OVERRIDE;
  void drawEdges(CGAL::Three::Viewer_interface *) const Q_DECL_OVERRIDE;
  void drawPoints(CGAL::Three::Viewer_interface *) const Q_DECL_OVERRIDE;
  std::vector<QColor>& color_vector();
  void set_patch_id(SMesh::Face_index f,int i)const;
  int patch_id(SMesh::Face_index f)const;
  bool supportsRenderingMode(RenderingMode m) const Q_DECL_OVERRIDE;
  bool isFinite() const Q_DECL_OVERRIDE { return true; }
  bool isEmpty() const Q_DECL_OVERRIDE;
  Bbox bbox() const Q_DECL_OVERRIDE;
  QString toolTip() const Q_DECL_OVERRIDE;

  // Only needed for Scene_polyhedron_item
  void setItemIsMulticolor(bool){} 
  void update_vertex_indices(){}
  void update_halfedge_indices(){}
  void update_facet_indices(){}

  Vertex_selection_map vertex_selection_map();
  Face_selection_map face_selection_map();

  std::vector<QColor>& color_vector();
  void set_patch_id(SMesh::Face_index f,int i)const;
  int patch_id(SMesh::Face_index f)const;
  bool supportsRenderingMode(RenderingMode m) const;

  SMesh* polyhedron();
  const SMesh* polyhedron() const;

  void compute_bbox()const Q_DECL_OVERRIDE;
  void standard_constructor(SMesh *sm);
  void invalidateOpenGLBuffers();
public Q_SLOTS:
  void itemAboutToBeDestroyed(Scene_item *) Q_DECL_OVERRIDE;
  virtual void selection_changed(bool) Q_DECL_OVERRIDE;
protected:
  friend struct Scene_surface_mesh_item_priv;
  Scene_surface_mesh_item_priv* d;
};
#if 0
Scene_surface_mesh_item::Halfedge_is_feature_map
get(halfedge_is_feature_t, Scene_surface_mesh_item::SMesh& smesh);

Scene_surface_mesh_item::Face_patch_id_map
get(face_patch_id_t, Scene_surface_mesh_item::SMesh& smesh);

Scene_surface_mesh_item::Vertex_selection_map
get(vertex_selection_t, Scene_surface_mesh_item::SMesh& smesh);

Scene_surface_mesh_item::Face_selection_map
get(face_selection_t, Scene_surface_mesh_item::SMesh& smesh);

Scene_surface_mesh_item::Vertex_num_feature_edges_map
get(vertex_num_feature_edges_t, Scene_surface_mesh_item::SMesh& smesh);
#else
Scene_surface_mesh_item::Halfedge_is_feature_map
inline get(halfedge_is_feature_t, Scene_surface_mesh_item::SMesh& smesh)
{
  typedef boost::graph_traits<Scene_surface_mesh_item::SMesh>::halfedge_descriptor halfedge_descriptor;
  return smesh.add_property_map<halfedge_descriptor,bool>("h:is_feature").first;
}


Scene_surface_mesh_item::Face_patch_id_map
inline get(face_patch_id_t, Scene_surface_mesh_item::SMesh& smesh)
{
  typedef  boost::graph_traits<Scene_surface_mesh_item::SMesh>::face_descriptor face_descriptor;
  return smesh.add_property_map<face_descriptor,int>("f:patch_id").first;
}


Scene_surface_mesh_item::Face_selection_map
inline get(face_selection_t, Scene_surface_mesh_item::SMesh& smesh)
{
  typedef  boost::graph_traits<Scene_surface_mesh_item::SMesh>::face_descriptor face_descriptor;
  return smesh.add_property_map<face_descriptor,int>("f:selection").first;
}


Scene_surface_mesh_item::Vertex_selection_map
inline get(vertex_selection_t, Scene_surface_mesh_item::SMesh& smesh)
{
  typedef  boost::graph_traits<Scene_surface_mesh_item::SMesh>::vertex_descriptor vertex_descriptor;
  return smesh.add_property_map<vertex_descriptor,int>("v:selection").first;
}

Scene_surface_mesh_item::Vertex_num_feature_edges_map
inline get(vertex_num_feature_edges_t, Scene_surface_mesh_item::SMesh& smesh)
{
  typedef  boost::graph_traits<Scene_surface_mesh_item::SMesh>::vertex_descriptor vertex_descriptor;
  return smesh.add_property_map<vertex_descriptor,int>("v:nfe").first;
}
#endif

namespace boost {
  
  template <>
  struct property_map<Scene_surface_mesh_item::SMesh, halfedge_is_feature_t>
  {
    typedef Scene_surface_mesh_item::SMesh SMesh;
    typedef boost::graph_traits<SMesh>::halfedge_descriptor halfedge_descriptor;
  
    typedef SMesh::Property_map<halfedge_descriptor, bool> type;
  };
  
  template <>
  struct property_map<Scene_surface_mesh_item::SMesh, face_patch_id_t>
  {
    typedef Scene_surface_mesh_item::SMesh SMesh;
    typedef boost::graph_traits<SMesh>::face_descriptor face_descriptor;
  
    typedef SMesh::Property_map<face_descriptor, int> type;
  }; 

  template <>
  struct property_map<Scene_surface_mesh_item::SMesh, vertex_selection_t>
  {
    typedef Scene_surface_mesh_item::SMesh SMesh;
    typedef boost::graph_traits<SMesh>::vertex_descriptor vertex_descriptor;
  
    typedef SMesh::Property_map<vertex_descriptor, int> type;
  };

  template <>
  struct property_map<Scene_surface_mesh_item::SMesh, face_selection_t>
  {
    typedef Scene_surface_mesh_item::SMesh SMesh;
    typedef boost::graph_traits<SMesh>::face_descriptor face_descriptor;
  
    typedef SMesh::Property_map<face_descriptor, int> type;
  };

  template <>
  struct property_map<Scene_surface_mesh_item::SMesh, vertex_num_feature_edges_t>
  {
    typedef Scene_surface_mesh_item::SMesh SMesh;
    typedef boost::graph_traits<SMesh>::vertex_descriptor vertex_descriptor;
  
    typedef SMesh::Property_map<vertex_descriptor, int> type;
  };

}



#endif /* CGAL_SCENE_SURFACE_MESH_ITEM_H */
