/**
 * You received this file as part of RRLib serialization
 *
 * Copyright (C) 2008-2011 Max Reichardt,
 *   Robotics Research Lab, University of Kaiserslautern
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __rrlib__serialization__tDataTypeBase_h__
#define __rrlib__serialization__tDataTypeBase_h__

#include <assert.h>
#include <boost/utility.hpp>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/type_traits/is_base_of.hpp>
#include <mutex>
#include "rrlib/logging/definitions.h"

namespace rrlib
{
namespace serialization
{
class tGenericObjectManager;
class tDataTypeAnnotation;
class tGenericObject;
class tFactory;
class tInputStream;
class tOutputStream;

/*!
 * \author Max Reichardt
 *
 * Untyped base class for all data types.
 *
 * Assigns unique type-id to each data type.
 * Can be used as factory for data types (necessary for deserializing)
 * vectors containing pointers).
 *
 * A instance of DataType<T> must be created for each type T
 * this mechanism should work with.
 *
 * This class is passed by value
 */
class tDataTypeBase
{
public:

  enum tType { ePLAIN, eLIST, ePTR_LIST, eNULL, eOTHER, eUNKNOWN };

public:

  static const size_t cMAX_ANNOTATIONS = 10;

  /*! Data type info */
  class tDataTypeInfoRaw : public boost::noncopyable
  {
  public:

    /*! Type of data type */
    tDataTypeBase::tType type;

    /*! Name of data type */
    std::string name;

    // RTTI name
    const char* rtti_name;

    // sizeof(T)
    size_t size;

    /*! New info? */
    bool new_info;

    /*! Is this the default name? - then it may be changed */
    bool default_name;

    /*! Data type uid */
    int16_t uid;

    /*! In case of list: type of elements */
    tDataTypeInfoRaw* element_type;

    /*! In case of element: list type (std::vector<T>) */
    tDataTypeInfoRaw* list_type;

    /*! In case of element: shared pointer list type (std::vector<std::shared_ptr<T>>) */
    tDataTypeInfoRaw* shared_ptr_list_type;

    /*! Annotations to data type */
    tDataTypeAnnotation* annotations[cMAX_ANNOTATIONS];

    /*!
     * \param placement (Optional) Destination for placement new
     * \return Instance of Datatype T casted to void*
     */
    virtual void* CreateInstance(void* placement = NULL) const
    {
      return NULL;
    }

    /*!
     * \param placement (Optional) Destination for placement new
     * \param manager_size Size of management info
     * \return Instance of Datatype as Generic object
     */
    virtual tGenericObject* CreateInstanceGeneric(void* placement = NULL, size_t manager_size = 0) const
    {
      return NULL;
    }

    /*!
     * Deep copy objects
     *
     * \param src Src object
     * \param dest Destination object
     * \param f Factory to use
     */
    virtual void DeepCopy(const void* src, void* dest, tFactory* f) const
    {
    }

    /*!
     * Deserialize object from input stream
     *
     * \param os InputStream
     * \param obj Object to deserialize
     */
    virtual void Deserialize(tInputStream& is, void* obj) const
    {
    }

    /*!
     * Serialize object to output stream
     *
     * \param os OutputStream
     * \param obj Object to serialize
     */
    virtual void Serialize(tOutputStream& os, const void* obj) const
    {
    }

    tDataTypeInfoRaw() :
        type(ePLAIN),
        name(),
        rtti_name(NULL),
        size(-1),
        new_info(true),
        default_name(true),
        uid(-1),
        element_type(NULL),
        list_type(NULL),
        shared_ptr_list_type(NULL)
    {
      for (size_t i = 0; i < cMAX_ANNOTATIONS; i++)
      {
        annotations[i] = NULL;
      }
    }

    virtual void Init() {}

    /*!
     * Set name of data type
     * (only valid if still default == not set before)
     *
     * \param new_name New name of type
     */
    void SetName(const std::string& new_name);

  };

private:

  /*! Log domain for this class */
  RRLIB_LOG_CREATE_NAMED_DOMAIN(log_domain, "serialization");

protected:

  //    /** Maximum number of types */
  //    public static final int MAX_TYPES = 2000;

  /*! Pointer to data type info (should not be copied every time for efficiency reasons) */
  const tDataTypeInfoRaw* info;

private:

  /*!
   * Helper for constructor (needs to be called in synchronized context)
   */
  void AddType(tDataTypeInfoRaw* nfo);

  template <typename T>
  struct tAnnotationIndex
  {
    static int index;
  };

  template <typename T>
  bool AnnotationIndexValid(bool set_valid = false)
  {
    static bool valid = false;
    if (set_valid)
    {
      assert(!valid);
      valid = true;
    }
    return valid;
  }

  inline const char* GetLogDescription()
  {
    return "DataTypeBase";
  }

  static std::recursive_mutex& GetMutex()
  {
    static std::recursive_mutex mutex;
    return mutex;
  }

  /*!
   * Helper method that safely provides static data type list
   */
  inline static std::vector<tDataTypeBase>& GetTypes()
  {
    static std::vector<tDataTypeBase> types;
    return types;
  }

public:

  /*!
   * \param name Name of data type
   */
  tDataTypeBase(tDataTypeInfoRaw* info_ = NULL);

  /*!
   * Add annotation to this data type
   *
   * \param ann Annotation
   */
  template <typename T>
  inline void AddAnnotation(T* ann)
  {
    std::unique_lock<std::recursive_mutex>(GetMutex());
    static size_t last_annotation_index = 0;
    if (info != NULL)
    {
      assert(((ann->annotated_type == NULL)) && "Already used as annotation in other object. Not allowed (double deleteting etc.)");
      ann->annotated_type = *this;
      size_t ann_index = -1u;

      if (!AnnotationIndexValid<T>())
      {
        last_annotation_index++;
        assert(last_annotation_index < cMAX_ANNOTATIONS);
        tAnnotationIndex<T>::index = last_annotation_index;
        AnnotationIndexValid<T>(true);
      }
      ann_index = tAnnotationIndex<T>::index;

      assert((ann_index > 0 && ann_index < cMAX_ANNOTATIONS));
      assert((info->annotations[ann_index] == NULL));

      const_cast<tDataTypeInfoRaw*>(info)->annotations[ann_index] = ann;
    }
    else
    {
      throw std::runtime_error("Null pointer !?");
    }
  }

  // Lookup data type by rtti name
  //
  // \param rtti_name rtti name
  // \return Data type with specified name (== NULL if it could not be found)
  static tDataTypeBase FindTypeByRtti(const char* rtti_name)
  {
    for (size_t i = 0; i < GetTypes().size(); i++)
    {
      if (GetTypes()[i].info->rtti_name == rtti_name)
      {
        return GetTypes()[i];
      }
    }
    return tDataTypeBase(NULL);
  }

  /*!
   * \param placement (Optional) Destination for placement new
   * \return Instance of Datatype T casted to void*
   */
  void* CreateInstance(void* placement = NULL) const;

  template < typename M = tGenericObjectManager >
  /*!
   * \param placement (Optional) Destination for placement new
   * \return Instance of Datatype as Generic object
   */
  inline tGenericObject* CreateInstanceGeneric(void* placement = NULL) const
  {
    if (info == NULL)
    {
      return NULL;
    }

    static const size_t cMANAGER_OFFSET = (sizeof(void*) == 4) ? 16 : 24; // must be identical to MANAGER_OFFSET in GenericObject

    static_assert(boost::is_base_of<tGenericObjectManager, M>::value, "only GenericObjectManagers allowed as M");
    tGenericObject* result = info->CreateInstanceGeneric(placement, sizeof(M));
    new(reinterpret_cast<char*>(result) + cMANAGER_OFFSET) M();
    return result;

  }

  // \return rtti name of data type
  const char* GetRttiName() const
  {
    if (info != NULL)
    {
      return info->rtti_name;
    }
    else
    {
      return typeid(void).name();
    }
  }

  // \return size of data type (as returned from sizeof(T))
  size_t GetSize() const
  {
    if (info != NULL)
    {
      return info->size;
    }
    else
    {
      return 0;
    }
  }

  // for checks against NULL (if (type == NULL) {...} )
  bool operator== (void* info_ptr) const
  {
    return info == info_ptr;
  }

  bool operator== (const tDataTypeBase& other) const
  {
    return info == other.info;
  }

  bool operator!= (void* info_ptr) const
  {
    return info != info_ptr;
  }

  bool operator!= (const tDataTypeBase& other) const
  {
    return info != other.info;
  }

  /*!
   * Deep copy objects
   *
   * \param src Src object
   * \param dest Destination object
   * \param f Factory to use
   */
  inline void DeepCopy(const void* src, void* dest, tFactory* f = NULL) const
  {
    if (info == NULL)
    {
      return;
    }
    info->DeepCopy(src, dest, f);
  }

  /*!
   * Deserialize object from input stream
   *
   * \param os InputStream
   * \param obj Object to deserialize
   */
  inline void Deserialize(tInputStream& is, void* obj) const
  {
    if (info == NULL)
    {
      return;
    }
    info->Deserialize(is, obj);
  }

  /*!
   * Lookup data type by name
   *
   * \param name Data Type name
   * \return Data type with specified name (NULL if it could not be found)
   */
  static tDataTypeBase FindType(const std::string& name);

  /*!
   * Get annotation of specified class
   *
   * \param c Class of annotation we're looking for
   * \return Annotation. Null if data type has no annotation of this type.
   */
  template <typename T>
  inline T* GetAnnotation()
  {
    if (info != NULL)
    {
      return static_cast<T*>(info->annotations[tAnnotationIndex<T>::index]);
    }
    else
    {
      throw std::runtime_error("Null pointer !?");
    }
  }

  /*!
   * Get uniform data type name from rtti type name
   *
   * \param rtti mangled rtti type name
   * \return Uniform data type name
   */
  static std::string GetDataTypeNameFromRtti(const char* rtti);

  /*!
   * \return In case of list: type of elements
   */
  inline tDataTypeBase GetElementType() const
  {
    if (info != NULL)
    {
      return tDataTypeBase(info->element_type);
    }
    return GetNullType();
  }

  /*!
   * \return DataTypeInfo object
   */
  inline const tDataTypeInfoRaw* GetInfo() const
  {
    return info;
  }

  /*!
   * \return In case of element: list type (std::vector<T>)
   */
  inline tDataTypeBase GetListType() const
  {
    if (info != NULL)
    {
      return tDataTypeBase(info->list_type);
    }
    return GetNullType();
  }

  /*!
   * \return Name of data type
   */
  inline const std::string& GetName() const
  {
    static const std::string unknown = "NULL";
    if (info != NULL)
    {
      return info->name;
    }
    return unknown;
  }

  /*!
   * \return Nulltype
   */
  inline static tDataTypeBase GetNullType()
  {
    return tDataTypeBase(NULL);
  }

  /*!
   * \return In case of element: shared pointer list type (std::vector<std::shared_ptr<T>>)
   */
  inline tDataTypeBase GetSharedPtrListType() const
  {
    if (info != NULL)
    {
      return tDataTypeBase(info->shared_ptr_list_type);
    }
    return GetNullType();
  }

  /*!
   * \return return "Type" of data type (see enum)
   */
  inline tDataTypeBase::tType GetType() const
  {
    if (info != NULL)
    {
      return info->type;
    }
    return eNULL;
  }

  /*!
   * \param uid Data type uid
   * \return Data type with specified uid
   */
  inline static tDataTypeBase GetType(int16_t uid)
  {
    if (uid == -1)
    {
      return GetNullType();
    }
    return GetTypes()[uid];
  }

  /*!
   * \return Number of registered types
   */
  inline static int16_t GetTypeCount()
  {
    return static_cast<int16_t>(GetTypes().size());
  }

  /*!
   * \return uid of data type
   */
  inline int16_t GetUid() const
  {
    if (info != NULL)
    {
      return info->uid;
    }
    return -1;
  }

  /*!
   * Can object of this data type be converted to specified type?
   * (In C++ currently only returns true, when types are equal)
   *
   * \param data_type Other type
   * \return Answer
   */
  inline bool IsConvertibleTo(const tDataTypeBase& data_type) const
  {
    return data_type == *this;
  }

  /*!
   * Serialize object to output stream
   *
   * \param os OutputStream
   * \param obj Object to serialize
   */
  inline void Serialize(tOutputStream& os, const void* obj) const
  {
    if (info == NULL)
    {
      return;
    }
    info->Serialize(os, obj);
  }

};

} // namespace rrlib
} // namespace serialization

namespace rrlib
{
namespace serialization
{
template <typename T>
int tDataTypeBase::tAnnotationIndex<T>::index;

} // namespace rrlib
} // namespace serialization

#endif // __rrlib__serialization__tDataTypeBase_h__
