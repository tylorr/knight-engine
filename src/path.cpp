#include <boost/filesystem/path.hpp>

/*
 * For some reason these methods are not being included in the msys2 build of
 * boost. I included it here so that the linker stops yelling at me. Ideally
 * I would find out why the msys2 package doesn't compile these functions even
 * though it is a windows build.
 */
namespace boost {
namespace filesystem {

const std::string path::generic_string() const
{
  path tmp(*this);
  std::replace(tmp.m_pathname.begin(), tmp.m_pathname.end(), L'\\', L'/');
  return tmp.string();
}

const std::string path::generic_string(const codecvt_type& cvt) const
{
  path tmp(*this);
  std::replace(tmp.m_pathname.begin(), tmp.m_pathname.end(), L'\\', L'/');
  return tmp.string(cvt);
}

const std::wstring path::generic_wstring() const
{ 
  path tmp(*this);
  std::replace(tmp.m_pathname.begin(), tmp.m_pathname.end(), L'\\', L'/');
  return tmp.wstring();
}

}
}
