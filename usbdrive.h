#ifndef __USB_DRIVE_H_
#define __USB_DRIVE_H_

#include <string>
#include <vector>

class Partition
{
  public:
    Partition() : name(""), size(0) { }
    Partition(const std::string _name, const std::size_t _size)
      : name(_name), size(_size) { }
    Partition(Partition const& partition) { (*this) = partition; }
    virtual ~Partition() { }
    Partition& operator=(Partition const& partition)
    {
      name = partition.name;
      size = partition.size;
      return (*this);
    }
    std::string getName() const         { return name; }
    void setName(const std::string val) { name = val;  }
    std::size_t getSize() const         { return size; }
    void setSize(const std::size_t val) { size = val;  }
    bool FindSize(std::string baseDev);
  protected:
    std::string name;
    std::size_t size;
};

class Disk : public Partition
{
  public:
    Disk() : Partition() { }
    Disk(const std::string _name, const std::size_t _size)
      : Partition(_name, _size) { }
    Disk(Disk const& disk) { (*this) = disk; }
    ~Disk() { }
    Disk& operator=(Disk const& disk)
    {
      Partition::operator=(disk);
      partitions = disk.partitions;
      return (*this);
    }
    bool FindPartitions();
    std::vector<Partition>& getPartitions() { return partitions; }
  protected:
    std::vector<Partition> partitions;
};

class USBDrive
{
  public:
    USBDrive() : strManufacturer("") , strProduct("") , strSerialNum("") { }
    USBDrive(USBDrive const& drv) { (*this) = drv; }
    ~USBDrive() { }
    USBDrive& operator=(USBDrive const& drv)
    {
      strManufacturer = drv.strManufacturer;
      strProduct      = drv.strProduct;
      strSerialNum    = drv.strSerialNum;
      disk            = drv.disk;
      return (*this);
    }
    std::string getManufacturer() const         { return strManufacturer; }
    void setManufacturer(const std::string val) { strManufacturer = val;  }
    std::string getProduct() const              { return strProduct;      }
    void setProduct(const std::string val)      { strProduct = val;       }
    std::string getSerialNum() const            { return strSerialNum;    }
    void setSerialNum(const std::string val)    { strSerialNum = val;     }
    std::string getFullname(const bool appendSerialNum = false) const;
    Disk& getDisk() { return disk; }
    static int FindStorageDrives(std::vector<USBDrive>& drives);
  protected:
    std::string strManufacturer;
    std::string strProduct;
    std::string strSerialNum;
    Disk disk;
};

#endif // __USB_DRIVE_H_
