from paraview.simple import *
import argparse

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument('--case', required=True)
args = parser.parse_args()

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')

case_vtk = LegacyVTKReader(FileNames=['/home/jcduenas/code/meshgeneration/Results/{case}/edgeTriangulation.vtk'.format(case=args.case)])

# set active source
SetActiveSource(case_vtk)

# get active source.
edgeTriangulationvtk = GetActiveSource()

# save data
SaveData('/home/jcduenas/Desktop/meshes/2D/{case}/{case}_binary.vtk'.format(case=args.case), proxy=edgeTriangulationvtk)