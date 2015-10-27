/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) 2015 NVIDIA
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtk_glew.h>
#include <vtkm/opengl/TransferToOpenGL.h>
#include "vtkmOpenGLVBO.h"

vtkStandardNewMacro(vtkmOpenGLVBO)

vtkmOpenGLVBO::vtkmOpenGLVBO()
{
  this->VertexCount = 0;
  this->Stride = 0;       // The size of a complete vertex + attributes
  this->VertexOffset = 0; // Offset of the vertex
  this->NormalOffset = 0; // Offset of the normal
  this->TCoordOffset = 0; // Offset of the texture coordinates
  this->TCoordComponents = 0; // Number of texture dimensions
  this->ColorOffset = 0;  // Offset of the color
  this->ColorComponents = 0; // Number of color components
  this->SetType(vtkOpenGLBufferObject::ArrayBuffer);
  // Don't bother GenBuffers'ing.  TransferToOpenGL will do that for us.
  this->vertices = this->normals = this->scalars = this->idx = 0;
  this->nidx = 0;
  // VTK probably has this somewhere, but creating our own is easier than
  // finding VTK's.
  glEnableClientState(GL_VERTEX_ARRAY);
  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);
}

vtkmOpenGLVBO::~vtkmOpenGLVBO()
{
  this->ReleaseGraphicsResources();
}

void vtkmOpenGLVBO::ReleaseGraphicsResources()
{
  if(this->vao == 0)
    {
    return;
    }

  glDeleteVertexArrays(1, &this->vao);
  this->vao = 0;

  if(this->vertices == 0)
    {
    return;
    }
  assert(this->normals != 0 && this->scalars != 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &this->vertices);
  glDeleteBuffers(1, &this->normals);
  glDeleteBuffers(1, &this->scalars);
  glDeleteBuffers(1, &this->idx);
}

// Unused.  Do not call!
void vtkmOpenGLVBO::AppendVBO(
  vtkPoints*, unsigned int,
  vtkDataArray*,
  vtkDataArray*,
  unsigned char*, int)
{
  abort();
}

// Do not use!
void vtkmOpenGLVBO::CreateVBO(
  vtkPoints *points, unsigned int numPts,
  vtkDataArray *normals,
  vtkDataArray *tcoords,
  unsigned char *colors, int colorComponents)
{
  abort();
}

size_t
vtkmOpenGLVBO::nindices() const {
  return this->nidx;
}

#include <fstream>
static void
write_obj(const std::string fname, const PyFRContourData::Vec3ArrayHandle& v) {
  { // don't overwrite an existing file.
    std::ifstream test(fname, std::ios::in);
    if(test.is_open()) {
      return;
    }
  }
  std::ofstream obj(fname.c_str(), std::ios::out);
  assert(obj);
  obj << "# tom's hacky vtkm vert writer\n";
  auto ctrl = v.GetPortalConstControl();
  for(size_t i=0; i < v.GetNumberOfValues(); ++i) {
    const vtkm::Vec<float,3> pt = ctrl.Get(vtkm::Id(i));
    obj << "v " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
  }

  for(size_t i=0; i < v.GetNumberOfValues()/3; ++i) {
    // obj indices start from 1, so +1.
    obj << "f " << i*3+0+1 << " " << i*3+1+1 << " " << i*3+2+1 << "\n";
  }
  obj.close();
}

void vtkmOpenGLVBO::Vertices(const PyFRContourData::Vec3ArrayHandle& vert)
{
  assert(this->vao != 0);
  if(this->vertices != 0)
    {
    std::ostringstream oss;
    oss << "You've already created this VBO; refusing to overwrite it with "
        << vert.GetNumberOfValues() << "-element vec-ArrayHandle.\n";
    throw std::runtime_error(oss.str());
    }
  // verts uninitialized implies indicies uninitialized.
  assert(this->idx == 0);
  glBindVertexArray(this->vao);
  vtkm::opengl::TransferToOpenGL(
    vert, this->vertices, vtkm::cont::DeviceAdapterTagCuda()
  );
  this->nidx = vert.GetNumberOfValues();

  // Generate triangle indices from all those points.
  glGenBuffers(1, &this->idx);
  assert(this->idx > 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idx);
  vtkm::cont::ArrayHandleCounting<vtkm::Id> indices(0,1,
                                                    vert.GetNumberOfValues());

  vtkm::opengl::TransferToOpenGL( indices, this->idx,
                                  vtkm::cont::DeviceAdapterTagCuda() );
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t),
  //             indices.data(), GL_STREAM_DRAW);
  write_obj("test.obj", vert);
}

void vtkmOpenGLVBO::Normals(const PyFRContourData::Vec3ArrayHandle& v)
{
  if(this->normals != 0)
    {
    std::ostringstream oss;
    oss << "You've already created this VBO; refusing to overwrite it with "
        << v.GetNumberOfValues() << "-element vec-ArrayHandle.";
    throw std::runtime_error(oss.str());
    }
  if(v.GetNumberOfValues() != this->nidx)
    {
    std::ostringstream err;
    err << "Trying to associate " << v.GetNumberOfValues() << " normals with "
        << this->nidx << " vertices does not make sense.";
    throw std::runtime_error(err.str());
    }
  glBindVertexArray(this->vao);
  vtkm::opengl::TransferToOpenGL(
    v, this->normals, vtkm::cont::DeviceAdapterTagCuda()
  );
}

void vtkmOpenGLVBO::Scalars(const PyFRContourData::ScalarDataArrayHandle& v)
{
  if(this->scalars != 0)
    {
    std::ostringstream oss;
    oss << "You've already created this VBO; refusing to overwrite it with "
        << v.GetNumberOfValues() << "-element ArrayHandle.\n";
    throw std::runtime_error(oss.str());
    }
  glBindVertexArray(this->vao);
  vtkm::opengl::TransferToOpenGL(
    v, this->scalars, vtkm::cont::DeviceAdapterTagCuda()
  );
}

bool vtkmOpenGLVBO::Bind()
{
  if(this->vertices == 0)
    {
    throw std::runtime_error("no vert VBO.  Did you forget to call "
                             "'Vertices(ArrayHandle)'?");
    }
  if(this->normals == 0)
    {
    throw std::runtime_error("no normals VBO.  Did you forget to call "
                             "'Normals(ArrayHandle)'?");
    }
  if(this->scalars == 0)
    {
    throw std::runtime_error("no scalars VBO.  Did you forget to call "
                             "'Scalars(ArrayHandle)'?");
    }
  glBindVertexArray(this->vao);
  const size_t threeD = 3;
  const size_t oneD = 1;
  const GLenum type = GL_FLOAT;
  const size_t stride = 0;

  glBindBuffer(GL_ARRAY_BUFFER, this->vertices);
  glVertexAttribPointer(0, threeD, type, GL_FALSE, stride, NULL);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, this->normals);
  glVertexAttribPointer(1, threeD, type, GL_FALSE, stride, NULL);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, this->scalars);
  glVertexAttribPointer(2, oneD, type, GL_FALSE, stride, NULL);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->idx);
  return true;
}

bool vtkmOpenGLVBO::Release()
{
  if(this->vertices == 0)
    {
    throw std::runtime_error("no vert VBO.  Did you forget to call "
                             "'Vertices(ArrayHandle)'?");
    }
  assert(this->idx != 0);
  if(this->normals == 0)
    {
    throw std::runtime_error("no normals VBO.  Did you forget to call "
                             "'Normals(ArrayHandle)'?");
    }
  if(this->scalars == 0)
    {
    throw std::runtime_error("no scalars VBO.  Did you forget to call "
                             "'Scalars(ArrayHandle)'?");
    }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindVertexArray(0);
  return true;
}


//-----------------------------------------------------------------------------
void vtkmOpenGLVBO::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
