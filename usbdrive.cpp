#include <fstream>
#include <blkid.h>
#include <libudev.h>
#include "usbdrive.h"

bool Partition::FindSize(std::string baseDev)
{
  // /sys/block/sdc/queue/logical_block_size    = 512
  std::string tmp(baseDev);
  std::size_t pos = tmp.find_last_of("/");
  tmp.erase(0, ++pos);
  std::string strSizePath("/sys/block/");
  strSizePath += tmp;
  strSizePath += "/queue/logical_block_size";
  std::ifstream fin(strSizePath);
  std::size_t blkSize = 0;
  fin >> blkSize;
  fin.close();

  std::string path("/sys/block/");
  path += tmp;
  if (baseDev != name) {
    // /sys/block/sdb/sdb1/size
    tmp = name;
    pos = tmp.find_last_of("/");
    tmp.erase(0, pos);
    path += tmp;
  }
  path += "/size";

  fin.open(path);
  std::size_t numBytes = 0;
  fin >> numBytes;
  fin.close();
  size = (numBytes * blkSize);
  return true;
}

bool Disk::FindPartitions()
{
  blkid_probe pr = blkid_new_probe_from_filename(name.c_str());
  if (!pr)
    return false;
  blkid_partlist ls = blkid_probe_get_partitions(pr);
  if (!ls) {
    // we're going to assume this is a disk with just a single main partition
    // so add the disk name (i.e. /dev/sdc) as a partition
    Partition p(name, 0);
    p.FindSize(name);
    partitions.push_back(p);
    blkid_free_probe(pr);
    return true;
  }
  int nparts = blkid_partlist_numof_partitions(ls);
  if (!nparts) {
    blkid_free_probe(pr);
    return false;
  }
  for (int i = 0; i < nparts; ++i) {
    blkid_partition par = blkid_partlist_get_partition(ls, i);
    int partNum = blkid_partition_get_partno(par);
    std::string strPart(name);
    strPart += std::to_string(partNum);
    Partition p(strPart, 0);
    p.FindSize(name);
    partitions.push_back(p);
  }
  ls = nullptr;
  blkid_free_probe(pr);
  pr = nullptr;
  return true;
}

static void trim(std::string &str)
{
  if (str.empty())
    return;
  const std::string delims(" \f\n\r\t\v");
  auto pos = str.find_first_not_of(delims);
  if (pos == str.npos) {
    str.clear();
    return;
  }
  if (pos != 0)
    str.erase(0, pos);
  pos = str.find_last_not_of(delims);
  if (pos != (str.length() - 1))
    str.erase((pos + 1), (str.length() - pos + 1));
}

static struct udev_device *get_child(struct udev *udev,
                                     struct udev_device *parent,
                                     const char *subsystem)
{
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_parent(enumerate, parent);
  udev_enumerate_add_match_subsystem(enumerate, subsystem);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

  struct udev_list_entry *entry = nullptr;
  struct udev_device *child = nullptr;
  udev_list_entry_foreach(entry, devices) {
    const char *path = udev_list_entry_get_name(entry);
    child = udev_device_new_from_syspath(udev, path);
    path = nullptr;
    break;
  }

  udev_enumerate_unref(enumerate);
  enumerate = nullptr;
  devices = nullptr;
  entry = nullptr;

  return child;
}

static void enumerate_usb_mass_storage(struct udev *udev,
                                       std::vector<USBDrive>& drives)
{
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_subsystem(enumerate, "scsi");
  udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

  struct udev_list_entry *entry = nullptr;
  udev_list_entry_foreach(entry, devices) {
    const char *path = udev_list_entry_get_name(entry);
    struct udev_device *scsi = udev_device_new_from_syspath(udev, path);

    struct udev_device *block = get_child(udev, scsi, "block");
    struct udev_device *scsi_disk = get_child(udev, scsi, "scsi_disk");
    struct udev_device *usb =
      udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

    if (block && scsi_disk && usb) {
      USBDrive ud;
      std::string str(udev_device_get_sysattr_value(usb, "manufacturer"));
      trim(str);
      ud.setManufacturer(str);
      str = udev_device_get_sysattr_value(usb, "product");
      trim(str);
      ud.setProduct(str);
      str = udev_device_get_sysattr_value(usb, "serial");
      trim(str);
      ud.setSerialNum(str);
      str = udev_device_get_devnode(block);
      trim(str);
      ud.getDisk().setName(str);
      if (ud.getDisk().FindPartitions())
        drives.push_back(ud);
    }

    if (block)
      udev_device_unref(block);

    if (scsi_disk)
      udev_device_unref(scsi_disk);

    udev_device_unref(scsi);

    path = nullptr;
    scsi = nullptr;
    block = nullptr;
    scsi_disk = nullptr;
    usb = nullptr;
  }

  udev_enumerate_unref(enumerate);

  enumerate = nullptr;
  devices = nullptr;
  entry = nullptr;
}

std::string USBDrive::getFullname(const bool appendSerialNum) const
{
  std::string str(strManufacturer);
  if ((!str.empty()) && (!strProduct.empty()))
    str += ' ';
  str += strProduct;
  if (appendSerialNum) {
    if ((!str.empty()) && (!strSerialNum.empty()))
      str += ' ';
    str += strSerialNum;
  }
  return str;
}

int USBDrive::FindStorageDrives(std::vector<USBDrive>& drives)
{
  drives.clear();

  struct udev *udev = udev_new();
  enumerate_usb_mass_storage(udev, drives);
  udev_unref(udev);
  udev = nullptr;

  return drives.size();
}
