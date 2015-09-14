#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include <vtkm/cont/DataSet.h>
#include "field.h"

class FieldData {
	public:
		FieldData(size_t n, void* fields);
		virtual ~FieldData();

		virtual void Coalesce();

	private:
		size_t n;
		struct field* fld;
		int32_t* types;
		vtkm::cont::DataSet ds;
};
#endif
