// This fixes a bug in SGI's libs, which manifests itself in missing
// symbol glPipelineInstrumentsBufferSGIX on machines other than Onyx IR/2's.
// The fix was sent to me by Tom Flynn. Tom, thanks!

#ifdef sgi

extern "C" {

#include <stdio.h>
#include <Performer/pf.h>
#include <Performer/pfdu.h>
#include <GL/gl.h>

// #pragma weak glPipelineInstrumentsBufferSGIX = glPipelineInstrumentsBufferSGIX_not_found

#if 0
void *mysymbol(void)
{
        pfNode *node;

        pfInit();
        pfMultiprocess(PFMP_DEFAULT);
        pfFilePath("/usr/share/Performer/data");
        pfdInitConverter("esprit.pfb");
        pfConfig();
        node = pfdLoadFile("esprit.pfb");
        return node;
}
#endif



#if 1
void glPipelineInstrumentsBufferSGIX(int a, GLfloat *b)
#else
void glPipelineInstrumentsBufferSGIX_not_found(int a, GLfloat *b)
#endif
{
    a = a;
    b = b;
    fprintf(stderr,
            "glPipelineInstrumentsBufferSGIX not linked in.\n");
}

}

#endif
