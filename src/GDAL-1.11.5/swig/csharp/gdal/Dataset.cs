/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace OSGeo.GDAL {

using System;
using System.Runtime.InteropServices;

public class Dataset : MajorObject {
  private HandleRef swigCPtr;

  public Dataset(IntPtr cPtr, bool cMemoryOwn, object parent) : base(GdalPINVOKE.DatasetUpcast(cPtr), cMemoryOwn, parent) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(Dataset obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
  public static HandleRef getCPtrAndDisown(Dataset obj, object parent) {
    if (obj != null)
    {
      obj.swigCMemOwn = false;
      obj.swigParentRef = parent;
      return obj.swigCPtr;
    }
    else
    {
      return new HandleRef(null, IntPtr.Zero);
    }
  }
  public static HandleRef getCPtrAndSetReference(Dataset obj, object parent) {
    if (obj != null)
    {
      obj.swigParentRef = parent;
      return obj.swigCPtr;
    }
    else
    {
      return new HandleRef(null, IntPtr.Zero);
    }
  }

  ~Dataset() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          GdalPINVOKE.delete_Dataset(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }
/*! Eight bit unsigned integer */ /*@SWIG:../include/csharp/gdal_csharp.i,93,%ds_rasterio_functions@*/
 public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, byte[] buffer, int buf_xSize, int buf_ySize, 
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = ReadRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Byte, 
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, byte[] buffer, int buf_xSize, int buf_ySize,
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = WriteRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Byte,
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  
/*@SWIG@*/
/*! Sixteen bit signed integer */ /*@SWIG:../include/csharp/gdal_csharp.i,93,%ds_rasterio_functions@*/
 public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, short[] buffer, int buf_xSize, int buf_ySize, 
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = ReadRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Int16, 
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, short[] buffer, int buf_xSize, int buf_ySize,
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = WriteRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Int16,
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  
/*@SWIG@*/
/*! Thirty two bit signed integer */ /*@SWIG:../include/csharp/gdal_csharp.i,93,%ds_rasterio_functions@*/
 public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, int[] buffer, int buf_xSize, int buf_ySize, 
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = ReadRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Int32, 
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, int[] buffer, int buf_xSize, int buf_ySize,
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = WriteRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Int32,
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  
/*@SWIG@*/
/*! Thirty two bit floating point */ /*@SWIG:../include/csharp/gdal_csharp.i,93,%ds_rasterio_functions@*/
 public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, float[] buffer, int buf_xSize, int buf_ySize, 
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = ReadRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Float32, 
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, float[] buffer, int buf_xSize, int buf_ySize,
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = WriteRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Float32,
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  
/*@SWIG@*/
/*! Sixty four bit floating point */ /*@SWIG:../include/csharp/gdal_csharp.i,93,%ds_rasterio_functions@*/
 public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, double[] buffer, int buf_xSize, int buf_ySize, 
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = ReadRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Float64, 
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, double[] buffer, int buf_xSize, int buf_ySize,
     int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
      CPLErr retval;
      GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
      try {
          retval = WriteRaster(xOff, yOff, xSize, ySize, handle.AddrOfPinnedObject(), buf_xSize, buf_ySize, DataType.GDT_Float64,
                               bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
      } finally {
          handle.Free();
      }
      GC.KeepAlive(this);
      return retval;
  }
  
/*@SWIG@*/

public int BuildOverviews( string resampling, int[] overviewlist, Gdal.GDALProgressFuncDelegate callback, string callback_data) {
      int retval;
      if (overviewlist.Length <= 0)
        throw new ArgumentException("overviewlist size is small (BuildOverviews)");
        
      IntPtr ptr = Marshal.AllocHGlobal(overviewlist.Length * Marshal.SizeOf(overviewlist[0]));
      try {
          Marshal.Copy(overviewlist, 0, ptr, overviewlist.Length);
          retval = BuildOverviews(resampling, overviewlist.Length, ptr, callback, callback_data);
      } finally {
          Marshal.FreeHGlobal(ptr);
      }
      GC.KeepAlive(this);
      return retval;
  }
public int BuildOverviews( string resampling, int[] overviewlist) {
      return BuildOverviews( resampling, overviewlist, null, null);
  }
  
public GCP[] GetGCPs() {
      /*hello*/
      IntPtr cPtr = __GetGCPs();
      int length = GetGCPCount();
      GCP[] ret = null;
      if (cPtr != IntPtr.Zero && length > 0)
      {
          ret = new GCP[length];
          for (int i=0; i < length; i++)
              ret[i] = __ReadCArrayItem_GDAL_GCP(cPtr, i);
      }
      GC.KeepAlive(this);
      return ret;
  }
  
public CPLErr SetGCPs(GCP[] pGCPs, string pszGCPProjection) {
     CPLErr ret = 0;
     if (pGCPs != null && pGCPs.Length > 0)
     {
         IntPtr cPtr = __AllocCArray_GDAL_GCP(pGCPs.Length);
         if (cPtr == IntPtr.Zero)
            throw new ApplicationException("Error allocating CArray with __AllocCArray_GDAL_GCP");
            
         try {
             for (int i=0; i < pGCPs.Length; i++)
                __WriteCArrayItem_GDAL_GCP(cPtr, i, pGCPs[i]);
             
             ret = __SetGCPs(pGCPs.Length, cPtr, pszGCPProjection);
         }
         finally
         {
            __FreeCArray_GDAL_GCP(cPtr);
         }
     }
     GC.KeepAlive(this);
     return ret;
  }
  public int RasterXSize {
    get {
      int ret = GdalPINVOKE.Dataset_RasterXSize_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public int RasterYSize {
    get {
      int ret = GdalPINVOKE.Dataset_RasterYSize_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public int RasterCount {
    get {
      int ret = GdalPINVOKE.Dataset_RasterCount_get(swigCPtr);
      if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public Driver GetDriver() {
    IntPtr cPtr = GdalPINVOKE.Dataset_GetDriver(swigCPtr);
    Driver ret = (cPtr == IntPtr.Zero) ? null : new Driver(cPtr, false, ThisOwn_false());
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public Band GetRasterBand(int nBand) {
    IntPtr cPtr = GdalPINVOKE.Dataset_GetRasterBand(swigCPtr, nBand);
    Band ret = (cPtr == IntPtr.Zero) ? null : new Band(cPtr, false, ThisOwn_false());
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetProjection() {
    string ret = GdalPINVOKE.Dataset_GetProjection(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetProjectionRef() {
    string ret = GdalPINVOKE.Dataset_GetProjectionRef(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public CPLErr SetProjection(string prj) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_SetProjection(swigCPtr, prj);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void GetGeoTransform(double[] argout) {
    GdalPINVOKE.Dataset_GetGeoTransform(swigCPtr, argout);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
  }

  public CPLErr SetGeoTransform(double[] argin) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_SetGeoTransform(swigCPtr, argin);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int BuildOverviews(string resampling, int overviewlist, IntPtr pOverviews, Gdal.GDALProgressFuncDelegate callback, string callback_data) {
    int ret = GdalPINVOKE.Dataset_BuildOverviews(swigCPtr, resampling, overviewlist, pOverviews, callback, callback_data);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int GetGCPCount() {
    int ret = GdalPINVOKE.Dataset_GetGCPCount(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetGCPProjection() {
    string ret = GdalPINVOKE.Dataset_GetGCPProjection(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void FlushCache() {
    GdalPINVOKE.Dataset_FlushCache(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
  }

  public CPLErr AddBand(DataType datatype, string[] options) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_AddBand(swigCPtr, (int)datatype, (options != null)? new GdalPINVOKE.StringListMarshal(options)._ar : null);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public CPLErr CreateMaskBand(int nFlags) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_CreateMaskBand(swigCPtr, nFlags);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string[] GetFileList() {
        /* %typemap(csout) char** CSL */
        IntPtr cPtr = GdalPINVOKE.Dataset_GetFileList(swigCPtr);
        IntPtr objPtr;
        int count = 0;
        if (cPtr != IntPtr.Zero) {
            while (Marshal.ReadIntPtr(cPtr, count*IntPtr.Size) != IntPtr.Zero)
                ++count;
        }
        string[] ret = new string[count];
        if (count > 0) {       
	        for(int cx = 0; cx < count; cx++) {
                objPtr = System.Runtime.InteropServices.Marshal.ReadIntPtr(cPtr, cx * System.Runtime.InteropServices.Marshal.SizeOf(typeof(IntPtr)));
                ret[cx]= (objPtr == IntPtr.Zero) ? null : System.Runtime.InteropServices.Marshal.PtrToStringAnsi(objPtr);
            }
        }
        if (cPtr != IntPtr.Zero)
            GdalPINVOKE.StringListDestroy(cPtr);
        
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
        return ret;
}

  public CPLErr ReadRaster(int xOff, int yOff, int xSize, int ySize, IntPtr buffer, int buf_xSize, int buf_ySize, DataType buf_type, int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_ReadRaster(swigCPtr, xOff, yOff, xSize, ySize, buffer, buf_xSize, buf_ySize, (int)buf_type, bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public CPLErr WriteRaster(int xOff, int yOff, int xSize, int ySize, IntPtr buffer, int buf_xSize, int buf_ySize, DataType buf_type, int bandCount, int[] bandMap, int pixelSpace, int lineSpace, int bandSpace) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset_WriteRaster(swigCPtr, xOff, yOff, xSize, ySize, buffer, buf_xSize, buf_ySize, (int)buf_type, bandCount, bandMap, pixelSpace, lineSpace, bandSpace);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private IntPtr __GetGCPs() {
      IntPtr ret = GdalPINVOKE.Dataset___GetGCPs(swigCPtr);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
}

  private CPLErr __SetGCPs(int nGCPs, IntPtr pGCPs, string pszGCPProjection) {
    CPLErr ret = (CPLErr)GdalPINVOKE.Dataset___SetGCPs(swigCPtr, nGCPs, pGCPs, pszGCPProjection);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void __WriteCArrayItem_GDAL_GCP(IntPtr carray, int index, GCP value) {
    GdalPINVOKE.Dataset___WriteCArrayItem_GDAL_GCP(swigCPtr, carray, index, GCP.getCPtr(value));
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
  }

  private GCP __ReadCArrayItem_GDAL_GCP(IntPtr carray, int index) {
    IntPtr cPtr = GdalPINVOKE.Dataset___ReadCArrayItem_GDAL_GCP(swigCPtr, carray, index);
    GCP ret = (cPtr == IntPtr.Zero) ? null : new GCP(cPtr, false, ThisOwn_false());
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private IntPtr __AllocCArray_GDAL_GCP(int size) {
      IntPtr ret = GdalPINVOKE.Dataset___AllocCArray_GDAL_GCP(swigCPtr, size);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
      return ret;
}

  private void __FreeCArray_GDAL_GCP(IntPtr carray) {
    GdalPINVOKE.Dataset___FreeCArray_GDAL_GCP(swigCPtr, carray);
    if (GdalPINVOKE.SWIGPendingException.Pending) throw GdalPINVOKE.SWIGPendingException.Retrieve();
  }

}

}
