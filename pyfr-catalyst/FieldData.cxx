#include <algorithm>
#include <iostream>
#include <iterator>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/Field.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/internal/DeviceAdapterTag.h>
// #include <vtkm/cont/cuda/internal/DeviceAdapterTagCuda.h>
#include <vtkm/worklet/DispatcherMapField.h>
// #include <vtkm/worklet/Magnitude.h>
#include <vtkm/worklet/ExternalFaces.h>
#include <vtkm/worklet/WorkletMapField.h>
#include "FieldData.h"

FieldData::FieldData(size_t nf, void* fields) :
	n(nf), fld(static_cast<struct field*>(fields)), types(NULL) {
	if(nf != 1) {
		throw std::runtime_error("We presently only handle 1 field.");
	}
	const size_t xyz_len = fld->nel*3;
	const size_t con_len = fld->na*fld->nb;
	const size_t off_len = fld->nb*8;
	const size_t typ_len = fld->nb*8;
	namespace vm = vtkm::cont;
	const float* vbuf = static_cast<const float*>(fld->verts);
	vm::ArrayHandle<float> vert = vm::make_ArrayHandle(vbuf, xyz_len);
	vm::ArrayHandle<int32_t> connectivity =
		vm::make_ArrayHandle(fld->con, con_len);
	vm::ArrayHandle<int32_t> offsets = vm::make_ArrayHandle(fld->off, off_len);

	// VTKm requires the type array to be its vtkm::Id type, which is 64bit by
	// default but 32bit with a special compile option.  We use that option
	// anyway because our connectivity etc. is 32bit.
	// Upcast into a new 32bit array.
	this->types = new int32_t[typ_len];
	for(size_t i=0; i < typ_len; ++i) {
		this->types[i] = (int32_t)fld->type[i];
	}
	vm::ArrayHandle<int32_t> types = vm::make_ArrayHandle(this->types, typ_len);

	enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
	vm::Field xyz("xyz", LINEAR, vm::Field::ASSOC_POINTS, vert);

	vm::CellSetExplicit<> cset(fld->nel, "cells", 3);
	cset.Fill(types, offsets, connectivity);

	this->ds.AddField(xyz);
	this->ds.AddCellSet(cset);
	// try {
	// 	this->glsetup();
	// } catch(const std::exception& e) {
	// 	std::cerr << e.what() << "\n";
	// }
}

FieldData::~FieldData() {
	delete[] this->types;
	this->types = NULL;
	// try {
	// 	this->glteardown();
	// } catch(const std::exception& e) {
	// 	std::cerr << e.what() << "\n";
	// }
}

// static void
// bindloc(const unsigned location, const unsigned bufid) {
// 	glBindBuffer(GL_ARRAY_BUFFER, bufid);
// 	glVertexAttribPointer(location, 3, GL_INT, GL_FALSE, 0, NULL);
// 	glEnableVertexAttribArray(location);
// }

// static void
// render(vtkm::cont::ArrayHandle<int32_t> vert,
//        vtkm::cont::ArrayHandle<int32_t> conn) {
// 	GLuint buffer[2];
// 	glGenBuffers(2, buffer);
//   vtkm::opengl::internal::TransferToOpenGL<int32_t,
// vtkm::cont::DeviceAdapterTagCuda> toGL;
//   toGL.Transfer(vert, buffer[0]);
//   toGL.Transfer(conn, buffer[1]);
// #if 0
// 	vtkm::opengl::TransferToOpenGL<VTKM_DEVICE_ADAPTER_CUDA>(
// 		vert, buffer[0], VTKM_DEVICE_ADAPTER_CUDA
// 	);
// 	vtkm::opengl::TransferToOpenGL(conn, buffer[1],
// 		vtkm::cont::DeviceAdapterTagCuda());
// #endif

// 	bindloc(0, buffer[0]);
// 	bindloc(1, buffer[1]);

// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 	glDrawArrays(GL_LINE_LOOP, 0, conn.GetNumberOfValues());

// 	glDisableVertexAttribArray(1);
// 	glDisableVertexAttribArray(0);
// 	glDeleteBuffers(2, buffer);
// }

void FieldData::Coalesce() {
	std::cout << "iso step on " << this->n << " fields.\n";
	const size_t nbytes = fld->nel * sizeof(float);
	std::shared_ptr<float> hsolution((float*)malloc(nbytes), free);
	if(hsolution == NULL) {
		throw std::runtime_error("alloc failure");
	}
	const cudaError_t err = cudaMemcpy(hsolution.get(), fld->solution, nbytes,
	                                   cudaMemcpyDeviceToHost);
	if(cudaSuccess != err) {
		throw std::runtime_error("memcpy failed: " + err);
	}

	namespace vm = vtkm::cont;
	enum ElemType { CONSTANT=0, LINEAR=1, QUADRATIC=2 };
	vm::Field rho("rho", LINEAR, vm::Field::ASSOC_POINTS, hsolution.get(),
	              fld->nel);
	this->ds.AddField(rho);

	vm::CellSetExplicit<>& cset =
		this->ds.GetCellSet(0).CastTo<vtkm::cont::CellSetExplicit<>>();
	vm::ArrayHandle<vtkm::Id> shapes = cset.GetShapesArray();
	vm::ArrayHandle<vtkm::Id> nindices = cset.GetNumIndicesArray();
	vm::ArrayHandle<vtkm::Id> conn = cset.GetConnectivityArray();
	vm::ArrayHandle<vtkm::Id> shapes_out;
	vm::ArrayHandle<vtkm::Id> nindices_out;
	vm::ArrayHandle<vtkm::Id> conn_out;

	vtkm::worklet::ExternalFaces<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>().run(
		shapes, nindices, conn, shapes_out, nindices_out, conn_out
	);
	vm::DataSet outds;
	for(size_t i=0; i < (size_t)ds.GetNumberOfCoordinateSystems(); ++i) {
		outds.AddCoordinateSystem(ds.GetCoordinateSystem(i));
	}

	std::cout << shapes.GetNumberOfValues() << " input elements, "
	          << shapes_out.GetNumberOfValues() << " output elements.\n";

	vm::CellSetExplicit<> outcset("cells", shapes_out.GetNumberOfValues());
	outcset.Fill(shapes_out, nindices_out, conn_out);
	outds.AddCellSet(outcset);

	// render(shapes_out, conn_out);
}
