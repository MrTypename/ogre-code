#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportTexture()
{
    class_<Texture, boost::noncopyable>("Texture", no_init);
}    

