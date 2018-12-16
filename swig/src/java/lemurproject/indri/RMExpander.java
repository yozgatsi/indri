/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package lemurproject.indri;
import java.util.Map;
public class RMExpander extends QueryExpander {
  private transient long swigCPtr;

  protected RMExpander(long cPtr, boolean cMemoryOwn) {
    super(indriJNI.RMExpander_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(RMExpander obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        indriJNI.delete_RMExpander(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public RMExpander(QueryEnvironment env, Map param) {
    this(indriJNI.new_RMExpander(QueryEnvironment.getCPtr(env), env, param), true);
  }

  public String expand(String originalQuery, ScoredExtentResult[] results) {
    return indriJNI.RMExpander_expand(swigCPtr, this, originalQuery, results);
  }

}
