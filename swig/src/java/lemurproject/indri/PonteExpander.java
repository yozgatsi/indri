/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package lemurproject.indri;
import java.util.Map;
public class PonteExpander extends QueryExpander {
  private transient long swigCPtr;

  protected PonteExpander(long cPtr, boolean cMemoryOwn) {
    super(indri_goJNI.PonteExpander_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(PonteExpander obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        indri_goJNI.delete_PonteExpander(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  public PonteExpander(QueryEnvironment env, Map param) {
    this(indri_goJNI.new_PonteExpander(QueryEnvironment.getCPtr(env), env, param), true);
  }

  public String expand(String originalQuery, ScoredExtentResult[] results) {
    return indri_goJNI.PonteExpander_expand(swigCPtr, this, originalQuery, results);
  }

}
