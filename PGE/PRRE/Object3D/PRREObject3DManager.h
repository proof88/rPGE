#pragma once

/*
    ###################################################################################
    PRREObject3DManager.h
    This file is part of PRRE.
    External header.
    PRREObject3DManager and PRREObject3D classes.
    Made by PR00F88
    EMAIL : PR0o0o0o0o0o0o0o0o0o0oF88@gmail.com
    ###################################################################################
*/


#include "../PRREallHeaders.h"
#include "../PRREFiledManager.h"
#include "PRREMesh3DManager.h"
#include "../Math/PRREVector.h"
#include "../../../../PFL/PFL/PFL.h"


/**
    Rotation order of 3D objects.
*/
enum TPRRE_ROTATION_ORDER
{
    PRRE_XYZ,
    PRRE_ZYX,
    PRRE_YXZ,
    PRRE_YZX,
    PRRE_XZY,
    PRRE_ZXY
}; // TPRRE_ROTATION_ORDER


/**
    Vertex Transfer Mode (TPRRE_VERTEX_TRANSFER_MODE) specifies how exactly the graphics pipeline is fed during geometry rendering.
    It is a simple number where each bit has different purpose.

    Automatically selected by PPP based on capabilities of the current hardware and the 2 PPP-settings:
    - vertex modifying habit (vmod),
    - vertex referencing mode (vref).

    Manual selection of the transfer mode is also available for custom purposes like testing, but keep in mind
    that manual selection may not take effect if the chosen mode is invalid for the current hardware and geometry data.

    It's not recommended to change transfer mode often during rendering, because it is a slow procedure:
    structure of geometry data may need to be reorganized and/or geometry data needs to be uploaded to host (video) memory.

    This setting is per-object, since different objects may need different transfer modes. For example, static geometry that is
    modified never or very rarely (e.g. buildings) should be placed in host/video memory, while other geometry changed often
    may be better to be in client/system memory, especially if the CPU is manipulating it instead of the GPU.
*/
typedef TPRREuint TPRRE_VERTEX_TRANSFER_MODE;


/**
    Vertex modifying habit (vmod) bit specifies how often the geometry data (vertices, normals, etc.) will be
    modified during the life of the application. This also specifies the primary location from where
    the graphics pipeline will be fed: if supported, all geometry data or some of it will be placed
    in server memory which is usually faster than client memory.
    This is a mandatory input setting of PPP.
*/
typedef TPRREuint TPRRE_VERTEX_MODIFYING_HABIT;

#define PRRE_VMOD_STATIC  0u                     /**< Vertices will be specified once and rarely or never modified. Faster. */
#define PRRE_VMOD_DYNAMIC BIT(PRRE_VT_VMOD_BIT)  /**< Vertices will be specified once and often or always modified. Slower. */


/**
    Vertex referencing mode (vref) specifies how the geometry data (vertices, normals, etc.) will be
    referenced while feeding them into the graphics pipeline.
    This is a mandatory input setting of PPP.
*/
typedef TPRREuint TPRRE_VERTEX_REFERENCING_MODE;

#define PRRE_VREF_DIRECT  0u                     /**< Vertex data will be fed into the pipeline by directly passing them.
                                                      Doesn't need an element (index) array to be specified but this disables
                                                      the use of shared vertices between primitives so this may be slower than indexed. */

#define PRRE_VREF_INDEXED BIT(PRRE_VT_VREF_BIT)  /**< Vertex data will be fed into the pipeline by indexing them.
                                                      Can be faster than direct feeding mode if the geometry is built up using
                                                      shared vertices, especially when shared vertices are referenced close to
                                                      each other (ordered). */

      
/* what purpose each bit stands for in TPRRE_VERTEX_TRANSFER_MODE */

#define PRRE_VT_VMOD_BIT    0      /**< vertex modifying habit (vmod) bit */
#define PRRE_VT_VREF_BIT    1      /**< vertex referencing mode (vref) bit */
#define PRRE_VT_VA_BIT      2      /**< passing vertices one-by-one (0) or in one shot as array (1) */
#define PRRE_VT_SVA_BIT     3      /**< regular vertex array (0) or server-side (1) */
#define PRRE_VT_CVA_BIT     4      /**< compiled array (0 no, 1 yes) */
#define PRRE_VT_RNG_BIT     5      /**< element array is ranged (0 no, 1 yes) */
#define PRRE_VT_VENDOR_BITS 6      /**< using generic solution (0) or vendor-specific (>0) */

/* 3 bits for vendor-specific starting from PRRE_VT_VENDOR_BITS */
#define PRRE_VT_VENDOR_GENERIC  0u
#define PRRE_VT_VENDOR_NVIDIA_1 1u
#define PRRE_VT_VENDOR_NVIDIA_2 2u
#define PRRE_VT_VENDOR_ATI_1    3u
#define PRRE_VT_VENDOR_ATI_2    4u
#define PRRE_VT_VENDOR_RSRVD_1  5u
#define PRRE_VT_VENDOR_RSRVD_2  6u
#define PRRE_VT_VENDOR_RSRVD_3  7u


/* predefined vertex transfer modes for convenience */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_1_BY_1             = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT; /**< SUPP! Basic direct immediate mode. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_RVA                = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT); /**< SUPP! Regular vertex array, drawArrays. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_RVA_CVA            = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_CVA_BIT); /**< SUPP! Regular vertex array, drawArrays, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_SVA_GEN            = PRRE_VMOD_DYNAMIC | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT); /**< SUPP! VBO, dynamic, array buffer. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_SVA_ATI            = PRRE_VT_DYN_DIR_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_1,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, dynamic. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_SVA_ATI_CVA        = PRRE_VT_DYN_DIR_SVA_ATI | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, dynamic, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_SVA_NV             = PRRE_VT_DYN_DIR_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_1,PRRE_VT_VENDOR_BITS,3); /**< NV VAR,  dynamic. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_DIR_SVA_NV_CVA         = PRRE_VT_DYN_DIR_SVA_NV | BIT(PRRE_VT_CVA_BIT); /**< NV VAR,  dynamic, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_1_BY_1             = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED; /**< SUPP! Basic indexed immediate mode. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_RVA                = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT); /**< SUPP! Regular vertex array, drawElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_RVA_RNG            = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< SUPP! Regular vertex array, drawRangeElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_RVA_CVA            = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_CVA_BIT); /**< SUPP! Regular vertex array, drawElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_RVA_CVA_RNG        = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< SUPP! Regular vertex array, drawRangeElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_GEN            = PRRE_VMOD_DYNAMIC | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT); /**< SUPP! VBO, dynamic, element buffer. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI            = PRRE_VT_DYN_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_1,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, dynamic, drawElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_RNG        = PRRE_VT_DYN_IND_SVA_ATI | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, dynamic, drawRangeElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_CVA        = PRRE_VT_DYN_IND_SVA_ATI | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, dynamic, drawElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_CVA_RNG    = PRRE_VT_DYN_IND_SVA_ATI | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, dynamic, drawRangeElements, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_EA         = PRRE_VT_DYN_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_2,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, dynamic, drawElementArrayATI. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_EA_RNG     = PRRE_VT_DYN_IND_SVA_ATI_EA | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, dynamic, drawRangeElementsArrayATI. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_EA_CVA     = PRRE_VT_DYN_IND_SVA_ATI_EA | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, dynamic, drawElementArrayATI, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_ATI_EA_CVA_RNG = PRRE_VT_DYN_IND_SVA_ATI_EA | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, dynamic, drawRangeElementsArrayATI, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV             = PRRE_VT_DYN_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_1,PRRE_VT_VENDOR_BITS,3); /**< NV VAR, dynamic, drawElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_RNG         = PRRE_VT_DYN_IND_SVA_NV | BIT(PRRE_VT_RNG_BIT); /**< NV VAR, dynamic, drawRangeElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_CVA         = PRRE_VT_DYN_IND_SVA_NV | BIT(PRRE_VT_CVA_BIT); /**< NV VAR, dynamic, drawElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_CVA_RNG     = PRRE_VT_DYN_IND_SVA_NV | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< NV VAR, dynamic, drawRangeElements, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_EA          = PRRE_VT_DYN_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_2,PRRE_VT_VENDOR_BITS,3); /**< NV VAR, dynamic, drawElementsNV. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_EA_RNG      = PRRE_VT_DYN_IND_SVA_NV_EA | BIT(PRRE_VT_RNG_BIT); /**< NV VAR, dynamic, drawRangeElementsNV. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_EA_CVA      = PRRE_VT_DYN_IND_SVA_NV_EA | BIT(PRRE_VT_CVA_BIT); /**< NV VAR, dynamic, drawElementsNV, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_DYN_IND_SVA_NV_EA_CVA_RNG  = PRRE_VT_DYN_IND_SVA_NV_EA | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< NV VAR, dynamic, drawRangeElementsNV, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_DL                 = PRRE_VMOD_STATIC  | PRRE_VREF_DIRECT; /**< SUPP! Display list built using direct data. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_SVA_GEN            = PRRE_VMOD_STATIC  | PRRE_VREF_DIRECT | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT); /**< SUPP! VBO, static, array buffer. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_SVA_ATI            = PRRE_VT_STA_DIR_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_1,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, static. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_SVA_ATI_CVA        = PRRE_VT_STA_DIR_SVA_GEN | BIT(PRRE_VT_CVA_BIT) | BITF_PREP(PRRE_VT_VENDOR_ATI_1,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, static, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_SVA_NV             = PRRE_VT_STA_DIR_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_1,PRRE_VT_VENDOR_BITS,3); /**< NV VAR,  static. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_DIR_SVA_NV_CVA         = PRRE_VT_STA_DIR_SVA_GEN | BIT(PRRE_VT_CVA_BIT) | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_1,PRRE_VT_VENDOR_BITS,3); /**< NV VAR,  static, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_DL                 = PRRE_VMOD_STATIC  | PRRE_VREF_INDEXED; /**< SUPP! Display list built using indexed data. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_GEN            = PRRE_VMOD_STATIC  | PRRE_VREF_INDEXED | BIT(PRRE_VT_VA_BIT) | BIT(PRRE_VT_SVA_BIT); /**< SUPP! VBO, static, element buffer. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI            = PRRE_VT_STA_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_1,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, static, drawElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_RNG        = PRRE_VT_STA_IND_SVA_ATI | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, static, drawRangeElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_CVA        = PRRE_VT_STA_IND_SVA_ATI | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, static, drawElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_CVA_RNG    = PRRE_VT_STA_IND_SVA_ATI | BIT(PRRE_VT_RNG_BIT) | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, static, drawRangeElements, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_EA         = PRRE_VT_STA_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_ATI_2,PRRE_VT_VENDOR_BITS,3); /**< ATI VAO, static, drawElementsATI. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_EA_RNG     = PRRE_VT_STA_IND_SVA_ATI_EA | BIT(PRRE_VT_RNG_BIT); /**< ATI VAO, static, drawRangeElementsATI. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_EA_CVA     = PRRE_VT_STA_IND_SVA_ATI_EA | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, static, drawElementsATI, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_ATI_EA_CVA_RNG = PRRE_VT_STA_IND_SVA_ATI_EA | BIT(PRRE_VT_RNG_BIT) | BIT(PRRE_VT_CVA_BIT); /**< ATI VAO, static, drawRangeElementsATI, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV             = PRRE_VT_STA_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_1,PRRE_VT_VENDOR_BITS,3); /**< NV VAR,  static, drawElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_RNG         = PRRE_VT_STA_IND_SVA_NV | BIT(PRRE_VT_RNG_BIT); /**< NV VAR,  static, drawRangeElements. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_CVA         = PRRE_VT_STA_IND_SVA_NV | BIT(PRRE_VT_CVA_BIT); /**< NV VAR,  static, drawElements, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_CVA_RNG     = PRRE_VT_STA_IND_SVA_NV | BIT(PRRE_VT_RNG_BIT) | BIT(PRRE_VT_CVA_BIT); /**< NV VAR,  static, drawRangeElements, compiled. */

const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_EA          = PRRE_VT_STA_IND_SVA_GEN | BITF_PREP(PRRE_VT_VENDOR_NVIDIA_2,PRRE_VT_VENDOR_BITS,3); /**< NV VAR,  static, drawElementsNV. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_EA_RNG      = PRRE_VT_STA_IND_SVA_NV_EA | BIT(PRRE_VT_RNG_BIT); /**< NV VAR,  static, drawRangeElementsNV. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_EA_CVA      = PRRE_VT_STA_IND_SVA_NV_EA | BIT(PRRE_VT_CVA_BIT); /**< NV VAR,  static, drawElementsNV, compiled. */
const TPRRE_VERTEX_TRANSFER_MODE PRRE_VT_STA_IND_SVA_NV_EA_CVA_RNG  = PRRE_VT_STA_IND_SVA_NV_EA | BIT(PRRE_VT_CVA_BIT) | BIT(PRRE_VT_RNG_BIT); /**< NV VAR,  static, drawRangeElementsNV, compiled. */


/**
    Array of transformed vertices stores the result XYZW-coordinates of vertices calculated by CPU after Vertex Processing stage.
    This is currently used by software renderer implementation only as hardware renderer does such calculation in GPU.
*/
struct TPRRE_TRANSFORMED_VERTEX
{
    TXYZW coords;
    TPRREbool draw;
};


class PRREObject3DManager;

/**
    3D object class.

    Object3D objects are 2-level entities:
     - first level (parent) derives from a level-1 Mesh3D object, and owns properties such as world position, angle, etc.;
     - second level objects derive from level-2 Mesh3D objects, particularly the submeshes of the level-1 mesh from where the level-1 Object3D is also derived.

    The biggest addition of Object3D to Mesh3D is the renderability.

    This class directly uses OpenGL.
*/
class PRREObject3D :
    public PRREMesh3D
{
#ifdef PRRE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PRREObject3D is included")
#endif

public:
    virtual ~PRREObject3D();

    PRREObject3D* getReferredObject() const;   /**< Gets the original object which was cloned to create this object. */

    TPRRE_VERTEX_MODIFYING_HABIT getVertexModifyingHabit() const;      /**< Gets vertex modifying habit. */
    void SetVertexModifyingHabit(TPRRE_VERTEX_MODIFYING_HABIT vmod);   /**< Sets vertex modifying habit. */
    TPRRE_VERTEX_REFERENCING_MODE getVertexReferencingMode() const;    /**< Gets vertex referencing mode. */
    void SetVertexReferencingMode(TPRRE_VERTEX_REFERENCING_MODE vref); /**< Sets vertex referencing mode. */
    TPRRE_VERTEX_TRANSFER_MODE getVertexTransferMode() const;          /**< Gets vertex transfer mode. */
    void SetVertexTransferMode(TPRRE_VERTEX_TRANSFER_MODE vtrans);     /**< Sets vertex transfer mode. */

    TPRRE_TRANSFORMED_VERTEX* getTransformedVertices(
        TPRREbool implicitAccessSubobject = true);                     /**< Gets the pointer to transformed vertices. */

          PRREVector& getAngleVec();                /**< Gets the rotation angles. */
    const PRREVector& getAngleVec() const;          /**< Gets the rotation angles. */

    virtual const PRREVector& getSizeVec() const;   /**< Gets the base sizes. */

          PRREVector  getScaledSizeVec() const;     /**< Gets the real sizes considering the geometry size calculated from vertex data and the current scaling factor. */

    const PRREVector& getScaling() const;           /**< Gets the scaling factor. */

    void  SetScaling(TPRREfloat value);             /**< Sets the scaling factor to given scalar. */
    void  SetScaling(const PRREVector& value);      /**< Sets the scaling factor to given vector. */

    void  Scale(TPRREfloat value);                  /**< Scales by the given scalar value. */
    void  Scale(const PRREVector& value);           /**< Scales by the given vector. */

    TPRREbool isVisible() const;                    /**< Gets the visibility state. */
    void      SetVisible(TPRREbool state);          /**< Sets the visibility state. */
    void      Show();                               /**< Sets the visibility state to true. */
    void      Hide();                               /**< Sets the visibility state to false. */
    TPRREbool isColliding_TO_BE_REMOVED() const;              /**< Gets whether colliding is enabled. */
    void      SetColliding_TO_BE_REMOVED(TPRREbool value);    /**< Sets whether colliding is enabled. */

    TPRRE_ROTATION_ORDER getRotationOrder() const;      /**< Gets the rotation order. */
    void SetRotationOrder(TPRRE_ROTATION_ORDER value);  /**< Sets the rotation order. */

    TPRREbool isLit() const;                              /**< Gets the lit state. */
    void      SetLit(TPRREbool value);                    /**< Sets the lit state. */
    TPRREbool isDoubleSided() const;                      /**< Gets the double sided state. */
    void      SetDoubleSided(TPRREbool value);            /**< Sets the double sided state. */
    TPRREbool isWireframed() const;                       /**< Gets the wireframed state. */
    void      SetWireframed(TPRREbool value);             /**< Sets the wireframed state. */
    TPRREbool isWireframedCulled() const;                 /**< Gets the wireframed culling state. */
    void      SetWireframedCulled(TPRREbool value);       /**< Sets the wireframed culling state. */
    TPRREbool isAffectingZBuffer() const;                 /**< Gets whether we write to the Z-Buffer while rendering. */
    void      SetAffectingZBuffer(TPRREbool value);       /**< Sets whether we write to the Z-Buffer while rendering. */
    TPRREbool isTestingAgainstZBuffer() const;            /**< Gets whether we test against the Z-Buffer while rendering. */
    void      SetTestingAgainstZBuffer(TPRREbool value);  /**< Sets whether we test against the Z-Buffer while rendering. */
    TPRREbool isStickedToScreen() const;                  /**< Gets the sticked-to-screen state. */
    void      SetStickedToScreen(TPRREbool value);        /**< Sets the sticked-to-screen state. */ 

    TPRREuint getUsedSystemMemory() const;    /**< Gets the amount of allocated system memory. */

    void Draw(bool bLighting);          /**< Draws the object. */

    // ---------------------------------------------------------------------------

protected:

    // ---------------------------------------------------------------------------
    
    PRREObject3D(
        const TPRRE_VERTEX_MODIFYING_HABIT& vmod = PRRE_VMOD_STATIC,
        const TPRRE_VERTEX_REFERENCING_MODE& vref = PRRE_VREF_DIRECT,
        TPRREbool bForceUseClientMemory = false);                        /**< Only PRREObject3DManager creates it. */
    
    PRREObject3D(const PRREObject3D&);                                   
    PRREObject3D& operator=(const PRREObject3D&);

private:
    class PRREObject3DImpl;
    PRREObject3DImpl* p;

    friend class PRREObject3DManager;  

}; // class PRREObject3D


/**
    3D object manager class.

    This class doesn't use any API directly, just constant values from OpenGL.
*/
class PRREObject3DManager :
    public PRREMesh3DManager
{
#ifdef PRRE_CLASS_IS_INCLUDED_NOTIFICATION
#pragma message("  PRREObject3DManager is included")
#endif

public:
    static TPRREbool isBlendFuncBlends(
        TPRRE_BLENDFACTORS sfactor,     
        TPRRE_BLENDFACTORS dfactor);    /**< Gets whether the given source and destination factors really mean blending or not. */

    static TPRREbool isBFB(
        TPRRE_BLENDFACTORS sfactor,                 
        TPRRE_BLENDFACTORS dfactor);    /**< Same as isBlendFuncBlends(). */

    /** Tells whether the given Vertex Transfer Mode is available on the current hardware. */
    static TPRREbool isVertexTransferModeSelectable(TPRRE_VERTEX_TRANSFER_MODE vtrans);

    /** Tells whether the given Vertex Transfer Mode references vertices by indexing. */
    static TPRREbool isVertexReferencingIndexed(TPRRE_VERTEX_TRANSFER_MODE vtrans);

    /** Tells whether the given Vertex Transfer Mode means dynamic modifying habit. */
    static TPRREbool isVertexModifyingDynamic(TPRRE_VERTEX_TRANSFER_MODE vtrans);

    static TPRRE_VERTEX_TRANSFER_MODE selectVertexTransferMode(
        TPRRE_VERTEX_MODIFYING_HABIT vmod,
        TPRRE_VERTEX_REFERENCING_MODE vref,
        TPRREbool bForceUseClientMemory );                    /**< Selects a suitable vertex transfer mode. */

    // ---------------------------------------------------------------------------

    PRREObject3DManager(PRRETextureManager& texMgr, PRREMaterialManager& matMgr);
    virtual ~PRREObject3DManager();

    TPRREbool isInitialized() const;  /**< Tells whether the object is correctly initialized or not. */
    
    PRREObject3D* createPlane(
        TPRREfloat a, TPRREfloat b,
        TPRRE_VERTEX_MODIFYING_HABIT vmod = PRRE_VMOD_STATIC,
        TPRRE_VERTEX_REFERENCING_MODE vref = PRRE_VREF_DIRECT,
        TPRREbool bForceUseClientMemory = false);                /**< Creates a new plane with the given sizes. */
    
    PRREObject3D* createBox(
        TPRREfloat a, TPRREfloat b, TPRREfloat c,
        TPRRE_VERTEX_MODIFYING_HABIT vmod = PRRE_VMOD_STATIC,
        TPRRE_VERTEX_REFERENCING_MODE vref = PRRE_VREF_DIRECT,
        TPRREbool bForceUseClientMemory = false);                /**< Creates a new box with the given sizes. */
    
    PRREObject3D* createCube(
        TPRREfloat a,
        TPRRE_VERTEX_MODIFYING_HABIT vmod = PRRE_VMOD_STATIC,
        TPRRE_VERTEX_REFERENCING_MODE vref = PRRE_VREF_DIRECT,
        TPRREbool bForceUseClientMemory = false);                /**< Creates a new cube with the given sizes. */

    PRREObject3D* createFromFile(
        const char* filename,
        TPRRE_VERTEX_MODIFYING_HABIT vmod,
        TPRRE_VERTEX_REFERENCING_MODE vref,
        TPRREbool bForceUseClientMemory = false);                /**< Creates object from the given file. */

    PRREObject3D* createFromFile(const char* filename);          /**< Creates object from the given file. */

    PRREObject3D* createCloned(PRREObject3D& referredobj);       /**< Creates a new object by cloning an already existing object. */

protected:

    PRREObject3DManager();

    PRREObject3DManager(const PRREObject3DManager&);
    PRREObject3DManager& operator=(const PRREObject3DManager&);

private:
    class PRREObject3DManagerImpl;
    PRREObject3DManagerImpl* p;  

}; // class PRREObject3DManager
