#pragma once

#pragma warning(disable:4530)
#include <uf_defs.h>
#include <uf_ui_types.h>
#include <uf.h>
#include <uf_modl.h>
#include <uf_obj.h>
#include <iostream>
#include <cmath>
#include <NXOpen/Xform.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/ListingWindow.hxx>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_Group.hxx>
#include <NXOpen/BlockStyler_SelectObject.hxx>
#include <NXOpen/BlockStyler_SpecifyPlane.hxx>
#include <NXOpen/BlockStyler_SpecifyOrientation.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Face.hxx>
#include <NXOpen/ConvergentFacet.hxx>
#include <NXOpen/TaggedObject.hxx>
#include <NXOpen/XformCollection.hxx>
#include <vector>
#include <string>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;

// Vector3d 操作的辅助函数声明
double Vector3dLength(const Vector3d& v);
Vector3d Vector3dNormalize(const Vector3d& v);
Vector3d Vector3dCross(const Vector3d& a, const Vector3d& b);

class DllExport AdjustEntity
{
public:
    static Session* theSession;
    static UI* theUI;
    AdjustEntity();
    ~AdjustEntity();
    NXOpen::BlockStyler::BlockDialog::DialogResponse Launch();
    void initialize_cb();
    void dialogShown_cb();
    int apply_cb();
    int ok_cb();
    int update_cb(NXOpen::BlockStyler::UIBlock* block);
    PropertyList* GetBlockProperties(const char* blockID);
private:
    const char* theDlxFileName;
    NXOpen::BlockStyler::BlockDialog* theDialog;
    NXOpen::BlockStyler::Group* group0;
    NXOpen::BlockStyler::SelectObject* selection;
    NXOpen::BlockStyler::Group* group;
    NXOpen::BlockStyler::SpecifyPlane* plane;
    NXOpen::BlockStyler::SpecifyOrientation* manip;
};

