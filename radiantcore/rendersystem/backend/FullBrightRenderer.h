#pragma once

#include "SceneRenderer.h"
#include "OpenGLStateManager.h"

namespace render
{

class FullBrightRenderer final :
    public SceneRenderer
{
private:
    const OpenGLStates& _sortedStates;
    IGeometryStore& _geometryStore;

public:
    FullBrightRenderer(RenderViewType renderViewType, const OpenGLStates& sortedStates, IGeometryStore& geometryStore) :
        SceneRenderer(renderViewType),
        _sortedStates(sortedStates),
        _geometryStore(geometryStore)
    {}

    IRenderResult::Ptr render(RenderStateFlags globalstate, const IRenderView& view, std::size_t time) override;
};

}
