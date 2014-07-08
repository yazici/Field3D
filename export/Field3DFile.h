//----------------------------------------------------------------------------//

/*
 * Copyright (c) 2014 Sony Pictures Imageworks Inc., 
 *                    Pixar Animation Studios Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.  Neither the name of Sony Pictures Imageworks nor the
 * names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//----------------------------------------------------------------------------//

/*! \file Field3DFile.h
  \brief Contains the Field3DFile classes
  \ingroup field

  OSS sanitized
*/

//----------------------------------------------------------------------------//

#ifndef _INCLUDED_Field3D_Field3DFile_H_
#define _INCLUDED_Field3D_Field3DFile_H_

//----------------------------------------------------------------------------//

#include <list>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "EmptyField.h"
#include "Field.h"
#include "FieldMetadata.h"
#include "ClassFactory.h"
#include "OgawaFwd.h"
#include "FieldCache.h"

//----------------------------------------------------------------------------//

#include "ns.h"

//----------------------------------------------------------------------------//

FIELD3D_NAMESPACE_OPEN

//----------------------------------------------------------------------------//
// Layer
//----------------------------------------------------------------------------//

//! Namespace for file I/O specifics
//! \ingroup file_int
namespace File {

/*! \class Layer
  \ingroup file_int
  This class wraps up information about a single "Layer" in a .f3d file.
  A layer is a "Field" with a name. The mapping lives on the Partition object,
  so the layer only knows about the location of the field in the file.
*/

class Layer
{
public:
  //! The name of the layer (always available)
  std::string name;
  //! The name of the parent partition. We need this in order to open
  //! its group.
  std::string parent;
};
  
} // namespace File

//----------------------------------------------------------------------------//
// Partition
//----------------------------------------------------------------------------//

namespace File {

/*! \class Partition
  \ingroup file_int
  This class represents the partition-level node in a f3D file. 
  The partition contains one "Mapping" and N "Fields" that all share that
  mapping.
*/

class FIELD3D_API Partition : public RefBase
{
public:

  typedef std::vector<Layer> ScalarLayerList;
  typedef std::vector<Layer> VectorLayerList;

  typedef boost::intrusive_ptr<Partition> Ptr;
  typedef boost::intrusive_ptr<const Partition> CPtr;

  // RTTI replacement ----------------------------------------------------------

  typedef Partition class_type;
  DEFINE_FIELD_RTTI_CONCRETE_CLASS;   
  
  static const char *staticClassType()
  {
    return "Partition";
  }

  // Ctors, dtor ---------------------------------------------------------------

  //! Ctor
  Partition() 
    : RefBase() 
  { }

  // From RefBase --------------------------------------------------------------

  //! \name From RefBase
  //! \{

  virtual std::string className() const;
  
  //! \}
  
  // Main methods --------------------------------------------------------------

  //! Adds a scalar layer
  void addScalarLayer(const File::Layer &layer);
  //! Adds a vector layer
  void addVectorLayer(const File::Layer &layer);

  //! Finds a scalar layer
  const File::Layer* scalarLayer(const std::string &name) const;
  //! Finds a vector layer
  const File::Layer* vectorLayer(const std::string &name) const;
  
  //! Gets all the scalar layer names. 
  void getScalarLayerNames(std::vector<std::string> &names) const;
  //! Gets all the vector layer names
  void getVectorLayerNames(std::vector<std::string> &names) const;

  // Public data members -------------------------------------------------------

  //! Name of the partition
  std::string name;
  //! Pointer to the mapping object.
  FieldMapping::Ptr mapping;

private:

  // Private data members ------------------------------------------------------

  //! The scalar-valued layers belonging to this partition
  ScalarLayerList m_scalarLayers;
  //! The vector-valued layers belonging to this partition
  VectorLayerList m_vectorLayers;

  // Typedefs ------------------------------------------------------------------

  //! Convenience typedef for referring to base class
  typedef RefBase base;

};

} // namespace File

//----------------------------------------------------------------------------//
// Field3DFileBase
//----------------------------------------------------------------------------//

/*! \class Field3DFileBase
  \ingroup file
  Provides some common functionality for Field3DInputFile and
  Field3DOutputFile. It hold the partition->layer data structures, but
  knows nothing about how to actually get them to/from disk. 
*/

//----------------------------------------------------------------------------//

class FIELD3D_API Field3DFileBase
{
public:

  // Structs -------------------------------------------------------------------

  struct LayerInfo 
  {
    std::string name;
    std::string parentName;
    int components;  
    LayerInfo(std::string par, std::string nm, int cpt) 
      : name(nm), parentName(par), components(cpt) 
    { /* Empty */ }
  };

  // Typedefs ------------------------------------------------------------------
 
  typedef std::map<std::string, std::string> GroupMembershipMap;

  // Ctor, dtor ----------------------------------------------------------------

  //! \name Constructors & destructor
  //! \{

  Field3DFileBase();
  //! Pure virtual destructor to ensure we never instantiate this class
  virtual ~Field3DFileBase() = 0;

  //! \}

  // Main methods --------------------------------------------------------------

  //! Clear the data structures and close the file.
  void clear();

  //! Closes the file. No need to call this unless you specifically want to
  //! close the file early. It will close once the File object goes out of 
  //! scope.
  bool close();

  //! \name Retreiving partition and layer names
  //! \{

  //! Gets the names of all the partitions in the file
  void getPartitionNames(std::vector<std::string> &names) const;
  //! Gets the names of all the scalar layers in a given partition
  void getScalarLayerNames(std::vector<std::string> &names, 
                           const std::string &partitionName) const;
  //! Gets the names of all the vector layers in a given partition
  void getVectorLayerNames(std::vector<std::string> &names, 
                           const std::string &partitionName) const;

  //! Returns a pointer to the given partition
  //! \returns NULL if no partition was found of that name
  File::Partition::Ptr getPartition(const std::string &partitionName) const
  { return partition(partitionName); }

  //! \}

  //! \name Convenience methods for partitionName
  //! \{

  //! Returns a unique partition name given the requested name. This ensures
  //! that partitions with matching mappings get the same name but each
  //! subsequent differing mapping gets a new, separate name
  std::string intPartitionName(const std::string &partitionName,
                               const std::string &layerName,
                               FieldRes::Ptr field);

  //! Strips any unique identifiers from the partition name and returns
  //! the original name
  std::string removeUniqueId(const std::string &partitionName) const;

  //! Add to the group membership
  void addGroupMembership(const GroupMembershipMap &groupMembers);

  //! \}

  // Access to metadata --------------------------------------------------------  

  //! accessor to the m_metadata class
  FieldMetadata<Field3DFileBase>& metadata()
  { return m_metadata; }

  //! Read only access to the m_metadata class
  const FieldMetadata<Field3DFileBase>& metadata() const
  { return m_metadata; }
 
  //! This function should implemented by concrete classes to  
  //! get the callback when metadata changes
  virtual void metadataHasChanged(const std::string &/* name */) 
  { /* Empty */ }

  // Debug ---------------------------------------------------------------------

  //! \name Debug
  //! \{

  void printHierarchy() const;

  //! \}

protected:

  // Internal typedefs ---------------------------------------------------------

  typedef std::vector<File::Partition::Ptr> PartitionList;
  typedef std::map<std::string, int> PartitionCountMap;

  // Convenience methods -------------------------------------------------------

  //! \name Convenience methods
  //! \{

  //! Closes the file if open.
  virtual void closeInternal() = 0;
  //! Returns a pointer to the given partition
  //! \returns NULL if no partition was found of that name
  File::Partition::Ptr partition(const std::string &partitionName);
  //! Returns a pointer to the given partition
  //! \returns NULL if no partition was found of that name
  File::Partition::Ptr partition(const std::string &partitionName) const;
  
  //! Gets the names of all the -internal- partitions in the file
  void getIntPartitionNames(std::vector<std::string> &names) const;
  //! Gets the names of all the scalar layers in a given partition, but
  //! assumes that partition name is the -internal- partition name
  void getIntScalarLayerNames(std::vector<std::string> &names, 
                              const std::string &intPartitionName) const;
  //! Gets the names of all the vector layers in a given partition, but
  //! assumes that partition name is the -internal- partition name
  void getIntVectorLayerNames(std::vector<std::string> &names, 
                              const std::string &intPartitionName) const;
  
  //! Returns the number of internal partitions for a given partition name
  int numIntPartitions(const std::string &partitionName) const;

  //! Makes an internal partition name given the external partition name.
  //! Effectively just tacks on .X to the name, where X is the number
  std::string makeIntPartitionName(const std::string &partitionsName,
                                   int i) const;

  //! \}

  // Data members --------------------------------------------------------------

  //! This stores layer info
  std::vector<LayerInfo> m_layerInfo;

  //! Vector of partitions. 
  PartitionList m_partitions;
  //! This stores partition names
  std::vector<std::string> m_partitionNames;

  //! Contains a counter for each partition name. This is used to keep multiple
  //! fields with the same name unique in the file
  PartitionCountMap m_partitionCount;

  //! Keeps track of group membership for each layer of partition name.
  //! The key is the "group" and the value is a space separated list of 
  //! "partitionName.0:Layer1 partitionName.1:Layer0  ..."  
  GroupMembershipMap m_groupMembership;

  //! metadata
  FieldMetadata<Field3DFileBase> m_metadata;

private:

  // Private member functions --------------------------------------------------

  Field3DFileBase(const Field3DFileBase&);
  void operator =(const Field3DFileBase&); 

};

//----------------------------------------------------------------------------//
// Field3DInputFile
//----------------------------------------------------------------------------//

/*! \class Field3DInputFile
  \brief Provides reading of .f3d (internally, Ogawa) files.
  \ingroup file

  Refer to \ref using_files for examples of how to use this in your code.

 */

//----------------------------------------------------------------------------//

class FIELD3D_API Field3DInputFile : public Field3DFileBase 
{
public:

  // Ctors, dtor ---------------------------------------------------------------

  //! \name Constructors & destructor
  //! \{

  Field3DInputFile();
  virtual ~Field3DInputFile();

  //! \}

  // From Field3DFileBase ------------------------------------------------------

  virtual void closeInternal()
  {
    m_archive.reset();
  }

  // Main interface ------------------------------------------------------------

  //! \name Reading layers from disk
  //! \{

  template <class Data_T>
  typename Field<Data_T>::Vec
  readLayers(const std::string &layerName = std::string("")) const;

  template <class Data_T>
  typename Field<Data_T>::Vec
  readLayers(const std::string &partitionName,
             const std::string &layerName) const;

  //! \name Backward compatibility
  //! \{

  //! Retrieves all the layers of scalar type and maintains their on-disk
  //! data types
  //! \param layerName If a string is passed in, only layers of that name will
  //! be read from disk.
  template <class Data_T>
  typename Field<Data_T>::Vec
  readScalarLayers(const std::string &layerName = std::string("")) const
  { return readLayers<Data_T>(layerName); }

  //! This one allows the allows the partitionName to be passed in
  template <class Data_T>
  typename Field<Data_T>::Vec
  readScalarLayers(const std::string &partitionName, 
                   const std::string &layerName) const
  { return readLayers<Data_T>(partitionName, layerName); }

  //! Retrieves all the layers of vector type and maintains their on-disk
  //! data types
  //! \param layerName If a string is passed in, only layers of that name will
  //! be read from disk.
  template <class Data_T>
  typename Field<FIELD3D_VEC3_T<Data_T> >::Vec
  readVectorLayers(const std::string &layerName = std::string("")) const
  { return readLayers<FIELD3D_VEC3_T<Data_T> >(layerName); }

  //! This version allows you to pass in the partition name
  template <class Data_T>
  typename Field<FIELD3D_VEC3_T<Data_T> >::Vec
  readVectorLayers(const std::string &partitionName, 
                   const std::string &layerName) const
  { return readLayers<FIELD3D_VEC3_T<Data_T> >(partitionName, layerName); }

  //! \}

  //! \name Reading proxy data from disk
  //! \{

  //! Retrieves a proxy version (EmptyField) of each layer .
  //! \note Although the call is templated, all fields are read, regardless
  //! of bit depth.
  //! \param name If a string is passed in, only layers of that name will
  //! be read from disk.
  template <class Data_T>
  typename EmptyField<Data_T>::Vec
  readProxyLayer(const std::string &partitionName, 
                 const std::string &layerName, 
                 bool isVectorLayer) const;

  //! Retrieves a proxy version (EmptyField) of each scalar layer 
  //! \note Although the call is templated, all fields are read, regardless
  //! of bit depth.
  //! \param name If a string is passed in, only layers of that name will
  //! be read from disk.
  template <class Data_T>
  typename EmptyField<Data_T>::Vec
  readProxyScalarLayers(const std::string &name = std::string("")) const;

  //! Retrieves a proxy version (EmptyField) of each vector layer 
  //! \note Although the call is templated, all fields are read, regardless
  //! of bit depth.
  //! \param name If a string is passed in, only layers of that name will
  //! be read from disk.
  template <class Data_T>
  typename EmptyField<Data_T>::Vec
  readProxyVectorLayers(const std::string &name = std::string("")) const;

  //! \}

  // File IO ---

  //! Opens the given file
  //! \returns Whether successful
  bool open(const std::string &filename);

private:

  // Convenience methods -------------------------------------------------------

  //! This call does the actual reading of a layer. Notice that it expects
  //! a unique -internal- partition name.
  template <class Data_T>
  typename Field<Data_T>::Ptr 
  readLayer(const std::string &intPartitionName, 
            const std::string &layerName) const;

  // Data members --------------------------------------------------------------

  //! Filename, only to be set by open().
  std::string m_filename;
  //! Pointer to the Ogawa archive
  boost::shared_ptr<Alembic::Ogawa::IArchive> m_archive;

};

//----------------------------------------------------------------------------//
// Utility functions
//----------------------------------------------------------------------------//

/*! \brief checks to see if a file/directory exists or not
  \param[in] filename the file/directory to check
  \retval true if it exists
  \retval false if it does not exist
*/
bool fileExists(const std::string &filename);

//----------------------------------------------------------------------------//
// Field3DOutputFile
//----------------------------------------------------------------------------//

/*! \class Field3DOutputFile
  \ingroup file
  \brief Provides writing of .f3d (internally, Ogawa) files.

  Refer to \ref using_files for examples of how to use this in your code.

 */

//----------------------------------------------------------------------------//

class FIELD3D_API Field3DOutputFile : public Field3DFileBase 
{
public:

  // Enums ---------------------------------------------------------------------

  enum CreateMode {
    OverwriteMode,
    FailOnExisting
  };

  // Ctors, dtor ---------------------------------------------------------------

  //! \name Constructors & destructor
  //! \{

  Field3DOutputFile();
  virtual ~Field3DOutputFile();

  //! \}

  // From Field3DFileBase ------------------------------------------------------

  virtual void closeInternal()
  {
    m_root.reset();
    m_archive.reset();
  }

  // Main interface ------------------------------------------------------------

  //! \name Writing layer to disk
  //! \{

  //! Writes a scalar layer to the "Default" partition.
  template <class Data_T>
  bool writeLayer(const std::string &layerName, 
                  typename Field<Data_T>::Ptr layer)
  { return writeLayer<Data_T>(std::string("default"), layerName, layer); }

  //! Writes a layer to a specific partition. The partition will be created if
  //! not specified.
  template <class Data_T>
  bool writeLayer(const std::string &partitionName, 
                  const std::string &layerName, 
                  typename Field<Data_T>::Ptr layer);

  //! Writes a layer to a specific partition. The field name and attribute
  //! name are used for partition and layer, respectively
  template <class Data_T>
  bool writeLayer(typename Field<Data_T>::Ptr layer)
  { return writeLayer<Data_T>(layer->name, layer->attribute, layer); }

  //! \}

  //! \name Backward compatibility
  //! \{

  //! Writes a scalar layer to the "Default" partition.
  template <class Data_T>
  bool writeScalarLayer(const std::string &layerName, 
                        typename Field<Data_T>::Ptr layer)
  { return writeScalarLayer<Data_T>(std::string("default"), layerName, layer); }

  //! Writes a layer to a specific partition. The partition will be created if
  //! not specified.
  template <class Data_T>
  bool writeScalarLayer(const std::string &partitionName, 
                        const std::string &layerName, 
                        typename Field<Data_T>::Ptr layer)
  { return writeLayer<Data_T>(partitionName, layerName, layer); }

  //! Writes a layer to a specific partition. The field name and attribute
  //! name are used for partition and layer, respectively
  template <class Data_T>
  bool writeScalarLayer(typename Field<Data_T>::Ptr layer)
  { return writeLayer<Data_T>(layer); }

  //! Writes a scalar layer to the "Default" partition.
  template <class Data_T>
  bool writeVectorLayer(const std::string &layerName, 
                        typename Field<FIELD3D_VEC3_T<Data_T> >::Ptr layer)
  { return writeVectorLayer<Data_T>(std::string("default"), layerName, layer); }

  //! Writes a layer to a specific partition. The partition will be created if
  //! not specified.
  template <class Data_T>
  bool writeVectorLayer(const std::string &partitionName, 
                        const std::string &layerName, 
                        typename Field<FIELD3D_VEC3_T<Data_T> >::Ptr layer)
  { return writeLayer<FIELD3D_VEC3_T<Data_T> >(partitionName, layerName, layer); }

  //! Writes a layer to a specific partition. The field name and attribute
  //! name are used for partition and layer, respectively
  template <class Data_T>
  bool writeVectorLayer(typename Field<FIELD3D_VEC3_T<Data_T> >::Ptr layer)
  { return writeLayer<FIELD3D_VEC3_T<Data_T> >(layer); }

  //! \}

  //! Creates a .f3d file on disk
  bool create(const std::string &filename, CreateMode cm = OverwriteMode);

  //! This routine is call if you want to write out global metadata to disk
  bool writeGlobalMetadata();

  //! This routine is called just before closing to write out any group
  //! membership to disk.
  bool writeGroupMembership();

private:
  
  // Convenience methods -------------------------------------------------------

  //! Increment the partition or make it zero if there's not an integer suffix
  std::string incrementPartitionName(std::string &pname);

  //! Create newPartition given the input config
  template <class Data_T>
  File::Partition::Ptr
  createNewPartition(const std::string &partitionName,
                     const std::string &layerName,
                     typename Field<Data_T>::Ptr field);
  //! Writes the mapping to the given Og node.
  //! Mappings are assumed to be light-weight enough to be stored as 
  //! plain attributes under a group.
  bool writeMapping(OgOGroup &partitionLocation, FieldMapping::Ptr mapping);
  
  //! Writes metadata for this layer
  bool writeMetadata(OgOGroup &metadataGroup, FieldBase::Ptr layer);

  //! Writes metadata for this file
  bool writeMetadata(OgOGroup &metadataGroup);

  // Data members --------------------------------------------------------------

  //! Pointer to the Ogawa archive
  boost::shared_ptr<Alembic::Ogawa::OArchive> m_archive;
  //! Pointer to root group
  boost::shared_ptr<OgOGroup> m_root;

};

//----------------------------------------------------------------------------//
// Implementations (Temp)
//----------------------------------------------------------------------------//

template <class Data_T>
bool Field3DOutputFile::writeLayer(const std::string &partitionName, 
                                   const std::string &layerName, 
                                   typename Field<Data_T>::Ptr layer)
{
  // Null pointer check
  if (!layer) {
    Msg::print(Msg::SevWarning,
               "Called writeLayer with null pointer. Ignoring...");
    return false;
  }
  
  // Make sure archive is open
  if (!m_archive) {
    Msg::print(Msg::SevWarning, 
               "Attempting to write layer without opening file first.");
    return false;
  }

  // Get the partition name
  // string partitionName = intPartitionName(userPartitionName, layerName, field);
 

  return true;
}

//----------------------------------------------------------------------------//

template <class Data_T>
typename Field<Data_T>::Vec
Field3DInputFile::readLayers(const std::string &name) const
{
  using std::vector;
  using std::string;

  typedef typename Field<Data_T>::Ptr FieldPtr;
  typedef typename Field<Data_T>::Vec FieldList;

  FieldList ret;
  std::vector<std::string> parts;
  getIntPartitionNames(parts);

  for (vector<string>::iterator p = parts.begin(); p != parts.end(); ++p) {
    vector<std::string> layers;
    getIntScalarLayerNames(layers, *p);
    for (vector<string>::iterator l = layers.begin(); l != layers.end(); ++l) {
      // Only read if it matches the name
      if ((name.length() == 0) || (*l == name)) {
        FieldPtr mf = readLayer<Data_T>(*p, *l);
        if (mf) {
          ret.push_back(mf);
        }
      }
    }
  }
  
  return ret;
}

//----------------------------------------------------------------------------//

template <class Data_T>
typename Field<Data_T>::Vec
Field3DInputFile::readLayers(const std::string &partitionName, 
                             const std::string &layerName) const
{
  using namespace std;
  
  typedef typename Field<Data_T>::Ptr FieldPtr;
  typedef typename Field<Data_T>::Vec FieldList;

  FieldList ret;

  if ((layerName.length() == 0) || (partitionName.length() == 0))
    return ret;
  
  std::vector<std::string> parts;
  getIntPartitionNames(parts);
 
  for (vector<string>::iterator p = parts.begin(); p != parts.end(); ++p) {
    std::vector<std::string> layers;
    getIntScalarLayerNames(layers, *p);
    if (removeUniqueId(*p) == partitionName) {
      for (vector<string>::iterator l = layers.begin(); 
           l != layers.end(); ++l) {
        // Only read if it matches the name
        if (*l == layerName) {
          FieldPtr mf = readLayer<Data_T>(*p, *l);
          if (mf)
            ret.push_back(mf);
        }
      }
    }
  }
  
  return ret;
}

//----------------------------------------------------------------------------//

template <class Data_T>
typename Field<Data_T>::Ptr
Field3DInputFile::readLayer(const std::string &intPartitionName,
                            const std::string &layerName) const
{
  return typename Field<Data_T>::Ptr();
}

//----------------------------------------------------------------------------//

FIELD3D_NAMESPACE_HEADER_CLOSE

//----------------------------------------------------------------------------//

#endif
