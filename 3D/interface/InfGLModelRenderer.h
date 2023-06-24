#pragma once
#include <tags/Tag.h>
#include <tags/handles/modeHandle.h>

#include <yttrium.h>
#include <yttriumGL.h>

struct render_geometry;
struct LOD_render_data;

#define INFGLMODELRENDERER_FLAGS_SET_UP (1<<15)

/*
 *
 * Certain fields in the tags are used by this
 *
 * pc_index/vertex_buffers:
 * 		ownsD3DResource:	0 if no GL buffer holds the data of this buffer, 1 if one does
 * 		m_resource: 		the GL name of the buffer (as a GLuint, so there are 4 bytes that could in theory be used for something else)
 * 		m_resourceView:		the Handle for the GLBufferResource
 *
 * 	LOD_render_data:
 * 		per_mesh_temporary_ent.block: 	repurposed to store the GL name of the VAO for this LOD (as a GLuint cast to a pointer)
 * 		flags_lod_render_flags:		The highest bit (1<<15) indicates whether or not the GL data for this lod has been set up
 */

class InfGLModelRenderer{
public:
	// create all required buffers and upload the data (mesh only, no materials)
	// returns the stride of the index buffer (which is needed to draw stuff)
	static int ensureBuffersForPart(render_geometry* geo_str, LOD_render_data* lodData, Tag* tag, ytr::ResourceManager* resourceManager);

	static void setupMesh(modeHandle* hndl, uint32_t meshIndex, uint32_t lod);

	// bind and unbind the VAO
	static void preDraw(LOD_render_data* lodData);
	static void postDraw();

	static void drawPart(render_geometry* geo_str, LOD_render_data* lodData, int part);
};
