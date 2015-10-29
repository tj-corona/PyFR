
#include "vtkPyFRContourMapper.h"

#include "vtkPYFrIndexBufferObject.h"
#include "vtkPYFrVertexBufferObject.h"

#include "vtkExecutive.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkPolyData.h"
#include "vtkRenderWindow.h"
#include "vtkStreamingDemandDrivenPipeline.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPyFRContourMapper)

//----------------------------------------------------------------------------
vtkPyFRContourMapper::vtkPyFRContourMapper():
  vtkOpenGLPolyDataMapper()
{
  this->ActiveContour = -1;
  this->ContourData = NULL;

  //Important override the default VBO with our own custom VBO Implementation
  //Likewise the same needs
  this->VBO->Delete();
  this->VBO = vtkPYFrVertexBufferObject::New();

  //Important we need to override the IBO with our own IBO classes
  this->Points.IBO->Delete();
  this->Tris.IBO->Delete();
  this->TrisEdges.IBO->Delete();

  this->Points.IBO = vtkPYFrIndexBufferObject::New();
  this->Tris.IBO = vtkPYFrIndexBufferObject::New();
  this->TrisEdges.IBO = vtkPYFrIndexBufferObject::New();
}

//----------------------------------------------------------------------------
void vtkPyFRContourMapper::SetInputData(vtkPyFRContourData *input)
{
  this->SetInputDataInternal(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkPyFRContourData *vtkPyFRContourMapper::GetInput()
{
  return vtkPyFRContourData::SafeDownCast(
    this->GetExecutive()->GetInputData(0, 0));
}

//----------------------------------------------------------------------------
int vtkPyFRContourMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPyFRContourData");
  return 1;
}

//-----------------------------------------------------------------------------
void vtkPyFRContourMapper::RenderPiece(vtkRenderer* ren, vtkActor *actor)
{
  // Make sure that we have been properly initialized.
  if (ren->GetRenderWindow()->CheckAbortStatus())
    {
    return;
    }

  this->ContourData = this->GetInput();

  if (this->ContourData == NULL)
    {
    vtkErrorMacro(<< "No input!");
    return;
    }

  this->InvokeEvent(vtkCommand::StartEvent,NULL);
  if (!this->Static)
    {
    this->GetInputAlgorithm()->Update();
    }
  this->InvokeEvent(vtkCommand::EndEvent,NULL);

  // if there are no points then we are done
  if (!this->CurrentInput->HasData())
    {
    return;
    }

  this->RenderPieceStart(ren, actor);
  this->RenderPieceDraw(ren, actor);
  this->RenderEdges(ren,actor);
  this->RenderPieceFinish(ren, actor);
}

//-----------------------------------------------------------------------------
void vtkPyFRContourMapper::RenderPieceStart(vtkRenderer* ren, vtkActor *actor)
{
  // Set the PointSize and LineWidget
#if GL_ES_VERSION_2_0 != 1
  glPointSize(actor->GetProperty()->GetPointSize()); // not on ES2
#endif

  vtkHardwareSelector* selector = ren->GetSelector();
  int picking = getPickState(ren);
  if (this->LastSelectionState != picking)
    {
    this->SelectionStateChanged.Modified();
    this->LastSelectionState = picking;
    }

  if (selector && this->PopulateSelectionSettings)
    {
    selector->BeginRenderProp();
    // render points for point picking in a special way
    if (selector->GetFieldAssociation() == vtkDataObject::FIELD_ASSOCIATION_POINTS &&
        selector->GetCurrentPass() >= vtkHardwareSelector::ID_LOW24)
      {
#if GL_ES_VERSION_2_0 != 1
      glPointSize(4.0); //make verts large enough to be sure to overlap cell
#endif
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0,2.0);  // supported on ES2/3/etc
      glDepthMask(GL_FALSE); //prevent verts from interfering with each other
      }
    if (selector->GetCurrentPass() == vtkHardwareSelector::COMPOSITE_INDEX_PASS)
      {
      selector->RenderCompositeIndex(1);
      }
    if (selector->GetCurrentPass() == vtkHardwareSelector::ID_LOW24 ||
        selector->GetCurrentPass() == vtkHardwareSelector::ID_MID24 ||
        selector->GetCurrentPass() == vtkHardwareSelector::ID_HIGH16)
      {
      selector->RenderAttributeId(0);
      }
    }

  this->TimeToDraw = 0.0;
  this->PrimitiveIDOffset = 0;

  // make sure the BOs are up to date
  this->UpdateBufferObjects(ren, actor);

  // Bind the OpenGL, this is shared between the different primitive/cell types.
  this->VBO->Bind();
  this->LastBoundBO = NULL;
}

//-----------------------------------------------------------------------------
void vtkPyFRContourMapper::RenderPieceDraw(vtkRenderer* ren, vtkActor *actor)
{
  // draw points
  if (this->Points.IBO->IndexCount)
    {
    // Update/build/etc the shader.
    this->UpdateShaders(this->ContourData, ren, actor);
    this->Points.IBO->Bind();
    glDrawRangeElements(GL_POINTS, 0,
                        static_cast<GLuint>(this->VBO->VertexCount - 1),
                        static_cast<GLsizei>(this->Points.IBO->IndexCount),
                        GL_UNSIGNED_INT,
                        reinterpret_cast<const GLvoid *>(NULL));
    this->Points.IBO->Release();
    this->PrimitiveIDOffset += (int)this->Points.IBO->IndexCount;
    }

  int representation = actor->GetProperty()->GetRepresentation();

  // render points for point picking in a special way
  // all cell types should be rendered as points
  vtkHardwareSelector* selector = ren->GetSelector();
  if (selector && this->PopulateSelectionSettings &&
      selector->GetFieldAssociation() == vtkDataObject::FIELD_ASSOCIATION_POINTS &&
      selector->GetCurrentPass() >= vtkHardwareSelector::ID_LOW24)
    {
    representation = VTK_POINTS;
    }

  vtkProperty *prop = actor->GetProperty();
  bool surface_offset =
    (this->GetResolveCoincidentTopology() || prop->GetEdgeVisibility())
    && prop->GetRepresentation() == VTK_SURFACE;

  if (surface_offset)
    {
    glEnable(GL_POLYGON_OFFSET_FILL);
    if ( this->GetResolveCoincidentTopology() == VTK_RESOLVE_SHIFT_ZBUFFER )
      {
      // do something rough is better than nothing
      double zRes = this->GetResolveCoincidentTopologyZShift(); // 0 is no shift 1 is big shift
      double f = zRes*4.0;
      glPolygonOffset(f + (prop->GetEdgeVisibility() ? 1.0 : 0.0),
        prop->GetEdgeVisibility() ? 1.0 : 0.0);  // supported on ES2/3/etc
      }
    else
      {
      double f, u;
      this->GetResolveCoincidentTopologyPolygonOffsetParameters(f,u);
      glPolygonOffset(f + (prop->GetEdgeVisibility() ? 1.0 : 0.0),
        u + (prop->GetEdgeVisibility() ? 1.0 : 0.0));  // supported on ES2/3/etc
      }
    }

  // draw polygons
  if (this->Tris.IBO->IndexCount)
    {
    // First we do the triangles, update the shader, set uniforms, etc.
    this->UpdateShaders(this->Tris, ren, actor);
    if (!this->HaveWideLines(ren,actor) && representation == VTK_WIREFRAME)
      {
      glLineWidth(actor->GetProperty()->GetLineWidth());
      }
    this->Tris.IBO->Bind();
    GLenum mode = (representation == VTK_POINTS) ? GL_POINTS :
      (representation == VTK_WIREFRAME) ? GL_LINES : GL_TRIANGLES;
    glDrawRangeElements(mode, 0,
                      static_cast<GLuint>(this->VBO->VertexCount - 1),
                      static_cast<GLsizei>(this->Tris.IBO->IndexCount),
                      GL_UNSIGNED_INT,
                      reinterpret_cast<const GLvoid *>(NULL));
    this->Tris.IBO->Release();
    this->PrimitiveIDOffset += (int)this->Tris.IBO->IndexCount/3;
    }

  if (selector && (
        selector->GetCurrentPass() == vtkHardwareSelector::ID_LOW24 ||
        selector->GetCurrentPass() == vtkHardwareSelector::ID_MID24 ||
        selector->GetCurrentPass() == vtkHardwareSelector::ID_HIGH16))
    {
    selector->RenderAttributeId(this->PrimitiveIDOffset);
    }
}

//-----------------------------------------------------------------------------
void vtkPyFRContourMapper::RenderEdges(vtkRenderer* ren, vtkActor *actor)
{
  vtkProperty *prop = actor->GetProperty();
  bool draw_surface_with_edges =
    (prop->GetEdgeVisibility() && prop->GetRepresentation() == VTK_SURFACE);

  if (!draw_surface_with_edges)
    {
    return;
    }

  this->DrawingEdges = true;

  // draw polygons
  if (this->TrisEdges.IBO->IndexCount)
    {
    // First we do the triangles, update the shader, set uniforms, etc.
    this->UpdateShaders(this->TrisEdges, ren, actor);
    if (!this->HaveWideLines(ren,actor))
      {
      glLineWidth(actor->GetProperty()->GetLineWidth());
      }
    this->TrisEdges.IBO->Bind();
    glDrawRangeElements(GL_LINES, 0,
                        static_cast<GLuint>(this->VBO->VertexCount - 1),
                        static_cast<GLsizei>(this->TrisEdges.IBO->IndexCount),
                        GL_UNSIGNED_INT,
                        reinterpret_cast<const GLvoid *>(NULL));
    this->TrisEdges.IBO->Release();
    }

  this->DrawingEdges = false;
}


//-----------------------------------------------------------------------------
void vtkPyFRContourMapper::RenderPieceFinish(vtkRenderer* ren,
  vtkActor *actor)
{
  vtkHardwareSelector* selector = ren->GetSelector();
  if (selector && this->PopulateSelectionSettings)
    {
    // render points for point picking in a special way
    if (selector->GetFieldAssociation() == vtkDataObject::FIELD_ASSOCIATION_POINTS &&
        selector->GetCurrentPass() >= vtkHardwareSelector::ID_LOW24)
      {
      glDepthMask(GL_TRUE);
      glDisable(GL_POLYGON_OFFSET_FILL);
      }
    selector->EndRenderProp();
    }

  if (this->LastBoundBO)
    {
    this->LastBoundBO->VAO->Release();
    }

  this->VBO->Release();

  vtkProperty *prop = actor->GetProperty();
  bool surface_offset =
    (this->GetResolveCoincidentTopology() || prop->GetEdgeVisibility())
    && prop->GetRepresentation() == VTK_SURFACE;
  if (surface_offset)
    {
    glDisable(GL_POLYGON_OFFSET_FILL);
    }

  // If the timer is not accurate enough, set it to a small
  // time so that it is not zero
  if (this->TimeToDraw == 0.0)
    {
    this->TimeToDraw = 0.0001;
    }

  this->UpdateProgress(1.0);
}

//----------------------------------------------------------------------------
// Update the network connected to this mapper.
void vtkPyFRContourMapper::Update(int port)
{
  if (this->Static)
    {
    return;
    }

  this->UpdateInformation();

  vtkInformation* inInfo = this->GetInputInformation();

  // If the estimated pipeline memory usage is larger than
  // the memory limit, break the current piece into sub-pieces.
  if (inInfo)
    {
    int currentPiece = this->NumberOfSubPieces * this->Piece;
    vtkStreamingDemandDrivenPipeline::SetUpdateExtent(
      inInfo,
      currentPiece,
      this->NumberOfSubPieces * this->NumberOfPieces,
      this->GhostLevel);
    }

  this->vtkMapper::Update(port);
}

//----------------------------------------------------------------------------
void vtkPyFRContourMapper::Update()
{
  this->Superclass::Update();
}

//----------------------------------------------------------------------------
int vtkPyFRContourMapper::ProcessRequest(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector*)
{
  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
    int currentPiece = this->NumberOfSubPieces * this->Piece;
    vtkStreamingDemandDrivenPipeline::SetUpdateExtent(
      inInfo,
      currentPiece,
      this->NumberOfSubPieces * this->NumberOfPieces,
      this->GhostLevel);
    }
  return 1;
}

//-------------------------------------------------------------------------
bool vtkPyFRContourMapper::GetNeedToRebuildBufferObjects(
  vtkRenderer *vtkNotUsed(ren), vtkActor *act)
{
  //this is only called from UpdateBufferObjects which is only called from
  //RenderPieceStart meaning that ContourData pointer will always be valid
  if (this->VBOBuildTime < this->GetMTime() ||
      this->VBOBuildTime < act->GetMTime() ||
      this->VBOBuildTime < this->ContourData->GetMTime() ||
      this->VBOBuildTime < this->SelectionStateChanged)
    {
    return true;
    }
  return false;
}

//-------------------------------------------------------------------------
void vtkPyFRContourMapper::BuildBufferObjects(vtkRenderer *ren, vtkActor *act)
{
  //Do not call MapScalars as that will cause bad things to happen
  //instead ContourData already has the rgba we need

  //we are always coloring by points
  this->HaveCellScalars = false;
  this->HaveCellNormals = false;

  // going to ignore the apple picking bug entirely
  this->HaveAppleBug = false;

  // rebuild the VBO if the data has changed
  if (this->VBOBuildTime < this->SelectionStateChanged ||
      this->VBOBuildTime < this->GetMTime() ||
      this->VBOBuildTime < act->GetMTime() ||
      this->VBOBuildTime < this->CurrentInput->GetMTime())
    {
    // dropping support for texture maps

    // Build the VBO using our new vtkPYFrVertexBufferObject
    dynamic_cast<vtkPYFrVertexBufferObject*>(this->VBO)->CreateVBO(this->ContourData);
    }

  // now create the IBOs
  if (
      this->VBOBuildTime < this->GetMTime() ||
      this->VBOBuildTime < this->CurrentInput->GetMTime() ||
      this->VBOBuildTime < act->GetProperty()->GetMTime() ||
      this->VBOBuildTime < this->SelectionStateChanged)
    {
    this->BuildIBO(ren, act, this->ContourData);
    }
}

//-------------------------------------------------------------------------
void vtkPyFRContourMapper::BuildIBO(
  vtkRenderer *ren,
  vtkActor *act,
  vtkPyFRContourData *contours)
{
  int representation = act->GetProperty()->GetRepresentation();

  pointsIBO = dynamic_cast<vtkPYFrIndexBufferObject*>(this->Points.IBO);
  trisIBO = dynamic_cast<vtkPYFrIndexBufferObject*>(this->Tris.IBO);
  triEdgesIBO = dynamic_cast<vtkPYFrIndexBufferObject*>(this->TrisEdges.IBO);

  pointsIBO->CreatePointIndexBuffer(contours);

  if (selector && this->PopulateSelectionSettings &&
      selector->GetFieldAssociation() == vtkDataObject::FIELD_ASSOCIATION_POINTS &&
      selector->GetCurrentPass() >= vtkHardwareSelector::ID_LOW24)
    {
    representation = VTK_POINTS;
    }

  if (representation == VTK_POINTS)
    {
    trisIBO->CreatePointIndexBuffer(contours);
    }
  else if (representation == VTK_WIREFRAME)
    {
    trisIBO->CreateTriangleLineIndexBuffer(contours);
    }
   else // SURFACE
    {
    trisIBO->CreateTriangleIndexBuffer(contours);
    }

  // when drawing edges also build the edge IBOs
  vtkProperty *prop = act->GetProperty();
  bool draw_surface_with_edges =
    (prop->GetEdgeVisibility() && prop->GetRepresentation() == VTK_SURFACE);
  if (draw_surface_with_edges)
    {
    triEdgesIBO->CreateTriangleLineIndexBuffer(contours);
    }
}


//----------------------------------------------------------------------------
void vtkPyFRContourMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
