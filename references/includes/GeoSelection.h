// (c) 2010 The Foundry Visionmongers Ltd.

#ifndef GEOSELECTION_H_
#define GEOSELECTION_H_

#include "DDImage/Hash.h"
#include "DDImage/OrderedMap.h"
//#include "DDImage/Sparse2DMatrix.h"

#include <vector>

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS) // cl compiler on window
#  pragma warning( push )
#  pragma warning( disable : 4251 ) // warning #4251: needs to have dll-interface to be used by clients of <X>
#endif // !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)

namespace DD {
  namespace Image {

    class GeoInfo;

    typedef std::vector<float> VertexSelection;
    typedef std::vector<float> FaceSelection;


    struct GeoInfoSelection {
      VertexSelection vertices;
      FaceSelection faces;
      bool object;

      GeoInfoSelection() :
        vertices(),
        faces(),
        object(false)
      {
      }


      void append(Hash& hash)
      {
        hash.append(vertices);
        hash.append(faces);
        hash.append(object);
      }
    };


    class DDImage_API GeoSelection : private OrderedMap<Hash, GeoInfoSelection*>
    {
      typedef OrderedMap<Hash, GeoInfoSelection*> Base;
      typedef Hash Key;
      typedef GeoInfoSelection* Value;

      mutable bool _cachedHashValid;
      mutable Hash _cachedHash;

      void invalidateCachedHash();

      template<typename T>
      friend class ReadWriteHandle;
    public:
      GeoSelection();
      ~GeoSelection();

      GeoSelection(const GeoSelection& other);
      GeoSelection& operator=(const GeoSelection& other);


      // Const accessors to contained objects
      bool                            hasKey(const Hash& objId) const;
      const GeoInfoSelection*         get(const Hash& objId) const;

      // indexed access
      size_t                          indexOf(const Hash& objId) const;
      const Hash&                     getKeyAt(size_t index) const;
      const GeoInfoSelection*         getAt(size_t index) const;

      // query the number of contained object selections
      size_t                          size() const;
      bool                            empty() const;

      /*! Set selection for an object id - transfers ownership of selection to GeoSelection */
      void set(const Hash& objId, GeoInfoSelection* selection);

      const VertexSelection& vertices(const Hash& objID) const;
      const FaceSelection& faces(const Hash& objID) const;
      bool objectSelected(const Hash& objID) const;

      /*! A handle for accessing a VertexSelection / FaceSelection 
          Will ensure any cached selection hashes are invalidated properly.
      */
      template<typename T>
      class ReadWriteHandle
      {
        GeoSelection* _parent;
        T*  _selection;
      public:
        ReadWriteHandle() : _parent(NULL), _selection(NULL)  {}
        ReadWriteHandle(GeoSelection* parent, T& selection) : _parent(parent), _selection(&selection) {}
        ~ReadWriteHandle() {if(_parent)_parent->invalidateCachedHash();}

        T& operator*() {return *_selection;}
        T* operator->() {return _selection;}

        const T& operator*() const {return *_selection;}
        const T* operator->() const {return _selection;}
      };

      /*! Obtain read/write access to the vertex selection array*/
      ReadWriteHandle<VertexSelection> verticesReadWrite(const Hash& objID);

      /*! Obtain read/write access to the face selection array*/
      ReadWriteHandle<FaceSelection> facesReadWrite(const Hash& objID);
      
      void setVertices(const Hash& objID, const VertexSelection& selection);
      void setFaces(const Hash& objID, const FaceSelection& selection);
      void setObjectSelected(const Hash& objID, bool selected);
      
      void clearVertices();
      void clearFaces();
      void clearObjects();

      void setAllVertices(float amount);
      void setAllFaces(float amount);
      void setAllObjects(bool selected);

      void append(Hash& hash) const;

      void addSelection(const GeoSelection& sel);
      void removeSelection(const GeoSelection& sel);

      void deleteAll();
      
      static Hash geoID(const GeoInfo& geo);
    };
    
    GeoSelection& GetGeometrySelection();

  } // namespace Image
} // namespace DD

#if !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)
#  pragma warning( pop ) 
#endif // !defined(FN_COMPILER_INTEL) && defined(FN_OS_WINDOWS)

#endif /* GEOSELECTION_H_ */
