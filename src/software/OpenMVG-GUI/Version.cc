#include "Version.hh"

namespace openMVG_gui
{

const int GuiVersion::CURRENT_VERSION_MAJOR    = GUIVersionMajorNumber();
const int GuiVersion::CURRENT_VERSION_MINOR    = GUIVersionMinorNumber();
const int GuiVersion::CURRENT_VERSION_REVISION = GUIVersionRevisionNumber();

const GuiVersion GuiVersion::CURRENT_VERSION = GuiVersion(
    GuiVersion::CURRENT_VERSION_MAJOR, GuiVersion::CURRENT_VERSION_MINOR, GuiVersion::CURRENT_VERSION_REVISION );

GuiVersion::GuiVersion( const int major, const int minor, const int revision )
    : m_major( major )
    , m_minor( minor )
    , m_revision( revision )
{
}

bool operator<( const GuiVersion &v1, const GuiVersion &v2 )
{
  if ( v1.m_major < v1.m_major )
  {
    return true;
  }
  else if ( v1.m_major > v2.m_major )
  {
    return false;
  }
  else
  {
    if ( v1.m_minor < v2.m_minor )
    {
      return true;
    }
    else if ( v1.m_minor > v2.m_minor )
    {
      return false;
    }
    else
    {
      return v1.m_revision < v2.m_revision;
    }
  }
}

bool operator<=( const GuiVersion &v1, const GuiVersion &v2 )
{
  return v1 < v2 || v1 == v2;
}

bool operator>( const GuiVersion &v1, const GuiVersion &v2 )
{
  return !( v1 <= v2 );
}
bool operator>=( const GuiVersion &v1, const GuiVersion &v2 )
{
  return !( v1 < v2 );
}
bool operator==( const GuiVersion &v1, const GuiVersion &v2 )
{
  return v1.m_major == v2.m_major && v1.m_minor == v2.m_minor && v1.m_revision == v2.m_revision;
}
bool operator!=( const GuiVersion &v1, const GuiVersion &v2 )
{
  return !( v1 == v2 );
}

} // namespace openMVG_gui
