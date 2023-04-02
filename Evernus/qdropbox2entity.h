#pragma once

/*!
  An interface class inherited by Dropbox entities (QDropbox2File, QDropbox2Folder,
  etc.) for homogeneous handling in containers.
 */
class IQDropbox2Entity
{
public:

    /*!
      Creates an empty instance of QDropbox2Entry.
      \param parent parent QObject
    */
    IQDropbox2Entity() {}

    /*!
       Creates a copy of an other QDropbox2Entry instance.

       \param other original instance
     */
    IQDropbox2Entity(const IQDropbox2Entity & /*other*/) {}

    /*!
      Default destructor. Takes care of cleaning up when the object is destroyed.
    */
    virtual ~IQDropbox2Entity() {}

    /*!
      Identifies the subclass instance as a folder or file.
    */
    virtual bool isDir() const = 0;
};
