
Welcome to the OpenWorlds v2.0R7 Merchant-SP VRML 97 loader!

Please make sure you read this entire README file before working with 
the Merchant-SP VRML 97 loader.

================
Important Notes:
================

1.  To use the loader you must have at least Version 3.2 of SGI's
    ImageVision EOE Libraries (used for processing ImageTextures),
    which is bundled with IRIX 6.3 and later.  For information about
    obtaining an update for the ImageVision runtime libraries, see:
    http://www.sgi.com/software/imagevision/faq.html
    http://www.sgi.com/Products/Evaluation/6.x_img_vision_3.2/


======================
Installing the loader:
======================

1.  The loader will be installed as part of your OpenGL
    Performer 2.4 installation, into the following default
    locations:

    /usr/lib/libpfdb/libpfwrl.so
    /usr/lib/libpfdb/.perfLoader.wrl
    /usr/lib32/libpfdb/libpfwrl.so
    /usr/lib32/libpfdb/.perfLoader.wrl
    /usr/lib64/libpfdb/libpfwrl.so
    /usr/lib64/libpfdb/.perfLoader.wrl

    You can then use the included VRML file to test the loader to see
    if it is working:  

    e.g.:  % perfly -W700,700 openworlds.wrl

    You should see something like this printed to the console, which 
    displays the product name, version string, and build date:
    =======================================================================
           OPENWORLDS MERCHANT(TM)-SP VRML 97 GEOMETRY LOADER v2.0R7
        for OpenGL Performer 2.4.0    Date: Nov  3 2000    DEMO VERSION
              Copyright(c) 2000 OpenWorlds.  All rights reserved.
    =======================================================================


2.  The license.dat file and LM_LICENSE_FILE:

    If you are using the full version of the Merchant-SP loader, you 
    will need to contact OpenWorlds to obtain a license.  When you recieve
    the license from us, make sure that you copy the license information
    exactly as it appears to a new or existing license.dat file.

    You will also need to set the LM_LICENSE_FILE environment variable to
    point to your license.dat file.  For example:

    setenv LM_LICENSE_FILE  /usr/bob/licenses/license.dat

    (The demo version of the loader does not require a license.)



==================
VRML Node Support:
==================

The following nodes are supported by the Merchant-SP VRML Loader at 
this time:

Appearance, Billboard*, Box, Collision, Color, Cone, Coordinate, 
Cylinder, DirectionalLight*, ElevationGrid*, Extrusion*, Fog*, 
FontStyle, Group, ImageTexture, IndexedFaceSet, IndexedLineSet, 
Inline*, LOD*, Material, Normal, PixelTexture*, PointLight*, PointSet, 
Shape, Sphere, SpotLight*, Switch, Text, TextureCoordinate, 
TextureTransform, Transform

Nodes marked with an asterisk(*) are supported in commercial version only.

All other nodes are ignored by the Merchant-SP VRML Loader.



=============
Contact Info:
=============

If you have any difficulty in using this loader, please email:

mailto:support@openworlds.com

If you wish to report a bug, please include your machine configuration,
full description of the problem, and a URL link or attached sample .wrl
file which demonstrates the problem.

For a complete VRML 97 browser with animation, web access, scripting
support in Java, JavaScript, and many other features, contact us at:
        
Tel: (215) 382-0390
Fax: (215) 382-0391
mailto:info@openworlds.com
http://www.openworlds.com

