#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# find source
clip = FindSource('Clip')

# set active source
SetActiveSource(clip)

# find source
contour = FindSource('Contour')

# set active source
SetActiveSource(contour)

# set active source
SetActiveSource(clip)

# set active source
SetActiveSource(contour)

# find source
slice = FindSource('Slice')

# set active source
SetActiveSource(slice)

# Properties modified on contour
contour.ContourField = 'Pressure'
contour.Isosurfaces = [1.899, 1.901]
contour.ColorField = 'Pressure'
contour.ColorPalette = 'Green-White Linear'
contour.ColorRange = [1.899, 1.901]

# Properties modified on clip
clip.Normal = [0.0, -1.0, 0.0]

# Properties modified on slice
slice.NumberOfPlanes = 1
slice.Spacing = 0.0
slice.Origin = [0.0, 0.0, 0.0]
slice.Normal = [0.0, 1.0, 0.0]
slice.ColorField = 'Pressure'
slice.ColorPalette = 'Green-White Linear'
slice.ColorRange = [1.899, 1.901]

#### uncomment the following to render all views
# RenderAllViews()
# alternatively, if you want to write images, you can use SaveScreenshot(...).