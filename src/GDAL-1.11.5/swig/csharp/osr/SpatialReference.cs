/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace OSGeo.OSR {

using System;
using System.Runtime.InteropServices;

public class SpatialReference : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;
  protected object swigParentRef;
  
  protected static object ThisOwn_true() { return null; }
  protected object ThisOwn_false() { return this; }

  public SpatialReference(IntPtr cPtr, bool cMemoryOwn, object parent) {
    swigCMemOwn = cMemoryOwn;
    swigParentRef = parent;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr(SpatialReference obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
  public static HandleRef getCPtrAndDisown(SpatialReference obj, object parent) {
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
  public static HandleRef getCPtrAndSetReference(SpatialReference obj, object parent) {
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

  ~SpatialReference() {
    Dispose();
  }

  public virtual void Dispose() {
  lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        OsrPINVOKE.delete_SpatialReference(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      swigParentRef = null;
      GC.SuppressFinalize(this);
    }
  }

  public SpatialReference(string wkt) : this(OsrPINVOKE.new_SpatialReference(wkt), true, null) {
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
  }

  public string __str__() {
    string ret = OsrPINVOKE.SpatialReference___str__(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsSame(SpatialReference rhs) {
    int ret = OsrPINVOKE.SpatialReference_IsSame(swigCPtr, SpatialReference.getCPtr(rhs));
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsSameGeogCS(SpatialReference rhs) {
    int ret = OsrPINVOKE.SpatialReference_IsSameGeogCS(swigCPtr, SpatialReference.getCPtr(rhs));
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsSameVertCS(SpatialReference rhs) {
    int ret = OsrPINVOKE.SpatialReference_IsSameVertCS(swigCPtr, SpatialReference.getCPtr(rhs));
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsGeographic() {
    int ret = OsrPINVOKE.SpatialReference_IsGeographic(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsProjected() {
    int ret = OsrPINVOKE.SpatialReference_IsProjected(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsCompound() {
    int ret = OsrPINVOKE.SpatialReference_IsCompound(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsGeocentric() {
    int ret = OsrPINVOKE.SpatialReference_IsGeocentric(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsLocal() {
    int ret = OsrPINVOKE.SpatialReference_IsLocal(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IsVertical() {
    int ret = OsrPINVOKE.SpatialReference_IsVertical(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int EPSGTreatsAsLatLong() {
    int ret = OsrPINVOKE.SpatialReference_EPSGTreatsAsLatLong(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int EPSGTreatsAsNorthingEasting() {
    int ret = OsrPINVOKE.SpatialReference_EPSGTreatsAsNorthingEasting(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetAuthority(string pszTargetKey, string pszAuthority, int nCode) {
    int ret = OsrPINVOKE.SpatialReference_SetAuthority(swigCPtr, pszTargetKey, pszAuthority, nCode);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetAttrValue(string name, int child) {
    string ret = OsrPINVOKE.SpatialReference_GetAttrValue(swigCPtr, name, child);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetAttrValue(string name, string value) {
    int ret = OsrPINVOKE.SpatialReference_SetAttrValue(swigCPtr, name, value);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetAngularUnits(string name, double to_radians) {
    int ret = OsrPINVOKE.SpatialReference_SetAngularUnits(swigCPtr, name, to_radians);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetAngularUnits() {
    double ret = OsrPINVOKE.SpatialReference_GetAngularUnits(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTargetLinearUnits(string target, string name, double to_meters) {
    int ret = OsrPINVOKE.SpatialReference_SetTargetLinearUnits(swigCPtr, target, name, to_meters);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLinearUnits(string name, double to_meters) {
    int ret = OsrPINVOKE.SpatialReference_SetLinearUnits(swigCPtr, name, to_meters);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLinearUnitsAndUpdateParameters(string name, double to_meters) {
    int ret = OsrPINVOKE.SpatialReference_SetLinearUnitsAndUpdateParameters(swigCPtr, name, to_meters);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetLinearUnits() {
    double ret = OsrPINVOKE.SpatialReference_GetLinearUnits(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetLinearUnitsName() {
    string ret = OsrPINVOKE.SpatialReference_GetLinearUnitsName(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetAuthorityCode(string target_key) {
    string ret = OsrPINVOKE.SpatialReference_GetAuthorityCode(swigCPtr, target_key);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetAuthorityName(string target_key) {
    string ret = OsrPINVOKE.SpatialReference_GetAuthorityName(swigCPtr, target_key);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetUTM(int zone, int north) {
    int ret = OsrPINVOKE.SpatialReference_SetUTM(swigCPtr, zone, north);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int GetUTMZone() {
    int ret = OsrPINVOKE.SpatialReference_GetUTMZone(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetStatePlane(int zone, int is_nad83, string unitsname, double units) {
    int ret = OsrPINVOKE.SpatialReference_SetStatePlane(swigCPtr, zone, is_nad83, unitsname, units);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int AutoIdentifyEPSG() {
    int ret = OsrPINVOKE.SpatialReference_AutoIdentifyEPSG(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetProjection(string arg) {
    int ret = OsrPINVOKE.SpatialReference_SetProjection(swigCPtr, arg);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetProjParm(string name, double val) {
    int ret = OsrPINVOKE.SpatialReference_SetProjParm(swigCPtr, name, val);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetProjParm(string name, double default_val) {
    double ret = OsrPINVOKE.SpatialReference_GetProjParm(swigCPtr, name, default_val);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetNormProjParm(string name, double val) {
    int ret = OsrPINVOKE.SpatialReference_SetNormProjParm(swigCPtr, name, val);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetNormProjParm(string name, double default_val) {
    double ret = OsrPINVOKE.SpatialReference_GetNormProjParm(swigCPtr, name, default_val);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetSemiMajor() {
    double ret = OsrPINVOKE.SpatialReference_GetSemiMajor(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetSemiMinor() {
    double ret = OsrPINVOKE.SpatialReference_GetSemiMinor(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public double GetInvFlattening() {
    double ret = OsrPINVOKE.SpatialReference_GetInvFlattening(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetACEA(double stdp1, double stdp2, double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetACEA(swigCPtr, stdp1, stdp2, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetAE(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetAE(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetBonne(double stdp, double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetBonne(swigCPtr, stdp, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetCEA(double stdp1, double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetCEA(swigCPtr, stdp1, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetCS(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetCS(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetEC(double stdp1, double stdp2, double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetEC(swigCPtr, stdp1, stdp2, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetEckertIV(double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetEckertIV(swigCPtr, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetEckertVI(double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetEckertVI(swigCPtr, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetEquirectangular(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetEquirectangular(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetEquirectangular2(double clat, double clong, double pseudostdparallellat, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetEquirectangular2(swigCPtr, clat, clong, pseudostdparallellat, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGaussSchreiberTMercator(double clat, double clong, double sc, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetGaussSchreiberTMercator(swigCPtr, clat, clong, sc, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGS(double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetGS(swigCPtr, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGH(double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetGH(swigCPtr, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetIGH() {
    int ret = OsrPINVOKE.SpatialReference_SetIGH(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGEOS(double cm, double satelliteheight, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetGEOS(swigCPtr, cm, satelliteheight, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGnomonic(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetGnomonic(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetHOM(double clat, double clong, double azimuth, double recttoskew, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetHOM(swigCPtr, clat, clong, azimuth, recttoskew, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetHOM2PNO(double clat, double dfLat1, double dfLong1, double dfLat2, double dfLong2, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetHOM2PNO(swigCPtr, clat, dfLat1, dfLong1, dfLat2, dfLong2, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetKrovak(double clat, double clong, double azimuth, double pseudostdparallellat, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetKrovak(swigCPtr, clat, clong, azimuth, pseudostdparallellat, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLAEA(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetLAEA(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLCC(double stdp1, double stdp2, double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetLCC(swigCPtr, stdp1, stdp2, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLCC1SP(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetLCC1SP(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLCCB(double stdp1, double stdp2, double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetLCCB(swigCPtr, stdp1, stdp2, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetMC(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetMC(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetMercator(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetMercator(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetMollweide(double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetMollweide(swigCPtr, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetNZMG(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetNZMG(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetOS(double dfOriginLat, double dfCMeridian, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetOS(swigCPtr, dfOriginLat, dfCMeridian, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetOrthographic(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetOrthographic(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetPolyconic(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetPolyconic(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetPS(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetPS(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetRobinson(double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetRobinson(swigCPtr, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetSinusoidal(double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetSinusoidal(swigCPtr, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetStereographic(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetStereographic(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetSOC(double latitudeoforigin, double cm, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetSOC(swigCPtr, latitudeoforigin, cm, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTM(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetTM(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTMVariant(string pszVariantName, double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetTMVariant(swigCPtr, pszVariantName, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTMG(double clat, double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetTMG(swigCPtr, clat, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTMSO(double clat, double clong, double scale, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetTMSO(swigCPtr, clat, clong, scale, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetVDG(double clong, double fe, double fn) {
    int ret = OsrPINVOKE.SpatialReference_SetVDG(swigCPtr, clong, fe, fn);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetWellKnownGeogCS(string name) {
    int ret = OsrPINVOKE.SpatialReference_SetWellKnownGeogCS(swigCPtr, name);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetFromUserInput(string name) {
    int ret = OsrPINVOKE.SpatialReference_SetFromUserInput(swigCPtr, name);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int CopyGeogCSFrom(SpatialReference rhs) {
    int ret = OsrPINVOKE.SpatialReference_CopyGeogCSFrom(swigCPtr, SpatialReference.getCPtr(rhs));
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetTOWGS84(double p1, double p2, double p3, double p4, double p5, double p6, double p7) {
    int ret = OsrPINVOKE.SpatialReference_SetTOWGS84(swigCPtr, p1, p2, p3, p4, p5, p6, p7);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int GetTOWGS84(double[] argout) {
    int ret = OsrPINVOKE.SpatialReference_GetTOWGS84(swigCPtr, argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetLocalCS(string pszName) {
    int ret = OsrPINVOKE.SpatialReference_SetLocalCS(swigCPtr, pszName);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGeogCS(string pszGeogName, string pszDatumName, string pszEllipsoidName, double dfSemiMajor, double dfInvFlattening, string pszPMName, double dfPMOffset, string pszUnits, double dfConvertToRadians) {
    int ret = OsrPINVOKE.SpatialReference_SetGeogCS(swigCPtr, pszGeogName, pszDatumName, pszEllipsoidName, dfSemiMajor, dfInvFlattening, pszPMName, dfPMOffset, pszUnits, dfConvertToRadians);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetProjCS(string name) {
    int ret = OsrPINVOKE.SpatialReference_SetProjCS(swigCPtr, name);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetGeocCS(string name) {
    int ret = OsrPINVOKE.SpatialReference_SetGeocCS(swigCPtr, name);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetVertCS(string VertCSName, string VertDatumName, int VertDatumType) {
    int ret = OsrPINVOKE.SpatialReference_SetVertCS(swigCPtr, VertCSName, VertDatumName, VertDatumType);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int SetCompoundCS(string name, SpatialReference horizcs, SpatialReference vertcs) {
    int ret = OsrPINVOKE.SpatialReference_SetCompoundCS(swigCPtr, name, SpatialReference.getCPtr(horizcs), SpatialReference.getCPtr(vertcs));
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromWkt(ref string ppszInput) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromWkt(swigCPtr, ref ppszInput);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromProj4(string ppszInput) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromProj4(swigCPtr, ppszInput);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromUrl(string url) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromUrl(swigCPtr, url);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromESRI(string[] ppszInput) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromESRI(swigCPtr, (ppszInput != null)? new OsrPINVOKE.StringListMarshal(ppszInput)._ar : null);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromEPSG(int arg) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromEPSG(swigCPtr, arg);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromEPSGA(int arg) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromEPSGA(swigCPtr, arg);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromPCI(string proj, string units, double[] argin) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromPCI(swigCPtr, proj, units, argin);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromUSGS(int proj_code, int zone, double[] argin, int datum_code) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromUSGS(swigCPtr, proj_code, zone, argin, datum_code);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromXML(string xmlString) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromXML(swigCPtr, xmlString);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromERM(string proj, string datum, string units) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromERM(swigCPtr, proj, datum, units);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromMICoordSys(string pszCoordSys) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromMICoordSys(swigCPtr, pszCoordSys);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ImportFromOzi(string datum, string proj, string projParms) {
    int ret = OsrPINVOKE.SpatialReference_ImportFromOzi(swigCPtr, datum, proj, projParms);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToWkt(out string argout) {
    int ret = OsrPINVOKE.SpatialReference_ExportToWkt(swigCPtr, out argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToPrettyWkt(out string argout, int simplify) {
    int ret = OsrPINVOKE.SpatialReference_ExportToPrettyWkt(swigCPtr, out argout, simplify);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToProj4(out string argout) {
    int ret = OsrPINVOKE.SpatialReference_ExportToProj4(swigCPtr, out argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToPCI(out string proj, out string units) {
    int ret = OsrPINVOKE.SpatialReference_ExportToPCI(swigCPtr, out proj, out units);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToUSGS(out int code, out int zone, out int datum) {
    int ret = OsrPINVOKE.SpatialReference_ExportToUSGS(swigCPtr, out code, out zone, out datum);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToXML(out string argout, string dialect) {
    int ret = OsrPINVOKE.SpatialReference_ExportToXML(swigCPtr, out argout, dialect);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int ExportToMICoordSys(out string argout) {
    int ret = OsrPINVOKE.SpatialReference_ExportToMICoordSys(swigCPtr, out argout);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SpatialReference CloneGeogCS() {
    IntPtr cPtr = OsrPINVOKE.SpatialReference_CloneGeogCS(swigCPtr);
    SpatialReference ret = (cPtr == IntPtr.Zero) ? null : new SpatialReference(cPtr, true, ThisOwn_true());
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public SpatialReference Clone() {
    IntPtr cPtr = OsrPINVOKE.SpatialReference_Clone(swigCPtr);
    SpatialReference ret = (cPtr == IntPtr.Zero) ? null : new SpatialReference(cPtr, true, ThisOwn_true());
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int Validate() {
    int ret = OsrPINVOKE.SpatialReference_Validate(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int StripCTParms() {
    int ret = OsrPINVOKE.SpatialReference_StripCTParms(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int FixupOrdering() {
    int ret = OsrPINVOKE.SpatialReference_FixupOrdering(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int Fixup() {
    int ret = OsrPINVOKE.SpatialReference_Fixup(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int MorphToESRI() {
    int ret = OsrPINVOKE.SpatialReference_MorphToESRI(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int MorphFromESRI() {
    int ret = OsrPINVOKE.SpatialReference_MorphFromESRI(swigCPtr);
    if (OsrPINVOKE.SWIGPendingException.Pending) throw OsrPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
