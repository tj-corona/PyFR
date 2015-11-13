#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# find source
clip = FindSource('Clip')
SetActiveSource(clip)

# Properties modified on clip
clip.Origin = [0.0, 0.0, 0.0]
clip.Normal = [0.0, -1.0, 0.0]

# find source
contour = FindSource('Contour')
SetActiveSource(contour)

# Properties modified on contour
contour.ContourField = 'Pressure'
contour.Isosurfaces = [1.899, 1.901]
contour.ColorField = 'Pressure'
contour.ColorPalette = 'Green-White Linear'
contour.ColorRange = [1.899, 1.901]


# find source
slice = FindSource('Slice')
SetActiveSource(slice)


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