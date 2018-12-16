/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package lemurproject.indri;

public class IndexStatus {
  private transient long swigCPtr;
  protected transient boolean swigCMemOwn;

  protected IndexStatus(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(IndexStatus obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        indri_goJNI.delete_IndexStatus(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  protected void swigDirectorDisconnect() {
    swigCMemOwn = false;
    delete();
  }

  public void swigReleaseOwnership() {
    swigCMemOwn = false;
    indri_goJNI.IndexStatus_change_ownership(this, swigCPtr, false);
  }

  public void swigTakeOwnership() {
    swigCMemOwn = true;
    indri_goJNI.IndexStatus_change_ownership(this, swigCPtr, true);
  }

  public void status(int code, String documentPath, String error, int documentsIndexed, int documentsSeen) {
    indri_goJNI.IndexStatus_status(swigCPtr, this, code, documentPath, error, documentsIndexed, documentsSeen);
  }

  public IndexStatus() {
    this(indri_goJNI.new_IndexStatus(), true);
    indri_goJNI.IndexStatus_director_connect(this, swigCPtr, swigCMemOwn, true);
  }

  public final static class action_code {
    public final static IndexStatus.action_code FileOpen = new IndexStatus.action_code("FileOpen");
    public final static IndexStatus.action_code FileSkip = new IndexStatus.action_code("FileSkip");
    public final static IndexStatus.action_code FileError = new IndexStatus.action_code("FileError");
    public final static IndexStatus.action_code FileClose = new IndexStatus.action_code("FileClose");
    public final static IndexStatus.action_code DocumentCount = new IndexStatus.action_code("DocumentCount");

    public final int swigValue() {
      return swigValue;
    }

    public String toString() {
      return swigName;
    }

    public static action_code swigToEnum(int swigValue) {
      if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
        return swigValues[swigValue];
      for (int i = 0; i < swigValues.length; i++)
        if (swigValues[i].swigValue == swigValue)
          return swigValues[i];
      throw new IllegalArgumentException("No enum " + action_code.class + " with value " + swigValue);
    }

    private action_code(String swigName) {
      this.swigName = swigName;
      this.swigValue = swigNext++;
    }

    private action_code(String swigName, int swigValue) {
      this.swigName = swigName;
      this.swigValue = swigValue;
      swigNext = swigValue+1;
    }

    private action_code(String swigName, action_code swigEnum) {
      this.swigName = swigName;
      this.swigValue = swigEnum.swigValue;
      swigNext = this.swigValue+1;
    }

    private static action_code[] swigValues = { FileOpen, FileSkip, FileError, FileClose, DocumentCount };
    private static int swigNext = 0;
    private final int swigValue;
    private final String swigName;
  }

}
