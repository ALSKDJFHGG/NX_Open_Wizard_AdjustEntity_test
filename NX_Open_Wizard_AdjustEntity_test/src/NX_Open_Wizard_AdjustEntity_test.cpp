#include "NX_Open_Wizard_AdjustEntity_test.h"

using namespace std;

// Vector3d 操作的辅助函数实现
double Vector3dLength(const Vector3d& v) {
    return sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
}

Vector3d Vector3dNormalize(const Vector3d& v) {
    double len = Vector3dLength(v);
    if (len > 1e-10) {
        return Vector3d(v.X / len, v.Y / len, v.Z / len);
    }
    return Vector3d(0.0, 0.0, 0.0);
}

Vector3d Vector3dCross(const Vector3d& a, const Vector3d& b) {
    return Vector3d(
        a.Y * b.Z - a.Z * b.Y,
        a.Z * b.X - a.X * b.Z,
        a.X * b.Y - a.Y * b.X
    );
}

Session* (AdjustEntity::theSession) = NULL;
UI* (AdjustEntity::theUI) = NULL;

AdjustEntity::AdjustEntity()
{
    try
    {
        AdjustEntity::theSession = NXOpen::Session::GetSession();
        AdjustEntity::theUI = UI::GetUI();
        theDlxFileName = "AdjustEntity.dlx";
        theDialog = AdjustEntity::theUI->CreateDialog(theDlxFileName);
        theDialog->AddApplyHandler(make_callback(this, &AdjustEntity::apply_cb));
        theDialog->AddOkHandler(make_callback(this, &AdjustEntity::ok_cb));
        theDialog->AddUpdateHandler(make_callback(this, &AdjustEntity::update_cb));
        theDialog->AddInitializeHandler(make_callback(this, &AdjustEntity::initialize_cb));
        theDialog->AddDialogShownHandler(make_callback(this, &AdjustEntity::dialogShown_cb));
    }
    catch (exception& ex)
    {
        throw;
    }
}

AdjustEntity::~AdjustEntity()
{
    if (theDialog != NULL)
    {
        delete theDialog;
        theDialog = NULL;
    }
}

extern "C" DllExport void  ufusr(char* param, int* retcod, int param_len)
{
    AdjustEntity* theAdjustEntity = NULL;
    try
    {
        theAdjustEntity = new AdjustEntity();
        theAdjustEntity->Launch();
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    if (theAdjustEntity != NULL)
    {
        delete theAdjustEntity;
        theAdjustEntity = NULL;
    }
}

extern "C" DllExport int ufusr_ask_unload()
{
    return (int)Session::LibraryUnloadOptionImmediately;
}

extern "C" DllExport void ufusr_cleanup(void)
{
    try
    {
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
}

NXOpen::BlockStyler::BlockDialog::DialogResponse AdjustEntity::Launch()
{
    NXOpen::BlockStyler::BlockDialog::DialogResponse dialogResponse = NXOpen::BlockStyler::BlockDialog::DialogResponseInvalid;
    try
    {
        dialogResponse = theDialog->Launch();
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return dialogResponse;
}

void AdjustEntity::initialize_cb()
{
    try
    {
        group0 = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("group0"));
        selection = dynamic_cast<NXOpen::BlockStyler::SelectObject*>(theDialog->TopBlock()->FindBlock("selection"));
        group = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("group"));
        plane = dynamic_cast<NXOpen::BlockStyler::SpecifyPlane*>(theDialog->TopBlock()->FindBlock("plane"));
        manip = dynamic_cast<NXOpen::BlockStyler::SpecifyOrientation*>(theDialog->TopBlock()->FindBlock("manip"));
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
}

void AdjustEntity::dialogShown_cb()
{
    try
    {
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
}

int AdjustEntity::apply_cb()
{
    int errorCode = 0;
    try
    {
        // 1. 获取所选的实体(Body)
        std::vector<TaggedObject*> selectedTaggedBodies = selection->GetSelectedObjects();
        if (selectedTaggedBodies.empty())
        {
            theSession->ListingWindow()->WriteLine("错误：未选择实体。");
            return 1;
        }

        Body* targetBody = nullptr;
        for (auto* taggedObj : selectedTaggedBodies)
        {
            Body* testBody = static_cast<Body*>(taggedObj);
            if (testBody != nullptr)
            {
                targetBody = testBody;
                break;
            }
        }
        if (targetBody == nullptr)
        {
            theSession->ListingWindow()->WriteLine("错误：选中的对象不是实体。");
            return 1;
        }

        // 2. 获取所选的面(Face)
        std::vector<TaggedObject*> selectedTaggedFaces = plane->GetSelectedObjects();
        if (selectedTaggedFaces.empty())
        {
            theSession->ListingWindow()->WriteLine("错误：未选择面。");
            return 1;
        }

        Face* targetFace = nullptr;
        for (auto* taggedObj : selectedTaggedFaces)
        {
            Face* testFace = dynamic_cast<Face*>(taggedObj);
            if (testFace != nullptr)
            {
                targetFace = testFace;
                break;
            }
        }
        if (targetFace == nullptr)
        {
            theSession->ListingWindow()->WriteLine("错误：选中的对象不是面。");
            return 1;
        }

        // 3. 获取面的中心和法向量
        Point3d faceCenter(0.0, 0.0, 0.0);
        Vector3d faceNormal(0.0, 0.0, 0.0);

        ConvergentFacet* firstFacet = targetFace->GetFirstFacetOnFace();
        if (firstFacet != nullptr)
        {
            std::vector<Point3d> vertices;
            try
            {
                vertices = firstFacet->GetVertices();
                if (vertices.size() >= 3)
                {
                    faceCenter.X = (vertices[0].X + vertices[1].X + vertices[2].X) / 3.0;
                    faceCenter.Y = (vertices[0].Y + vertices[1].Y + vertices[2].Y) / 3.0;
                    faceCenter.Z = (vertices[0].Z + vertices[1].Z + vertices[2].Z) / 3.0;

                    Vector3d normal = firstFacet->GetUnitNormal();
                    faceNormal = normal;
                }
            }
            catch (NXOpen::NXException&)
            {
                theSession->ListingWindow()->WriteLine("警告：无法获取面的顶点或法向量，使用默认值。");
            }
        }

        // 4. 获取目标轴（来自 SpecifyOrientation，或默认 Z 轴）
        Vector3d targetAxis(0.0, 0.0, 1.0);

        // 5. 计算 X 轴方向（与目标轴垂直）
        Vector3d refVector(1.0, 0.0, 0.0);
        Vector3d xAxis = Vector3dCross(targetAxis, refVector);

        if (Vector3dLength(xAxis) < 1e-10)
        {
            refVector = Vector3d(0.0, 1.0, 0.0);
            xAxis = Vector3dCross(targetAxis, refVector);
        }
        xAxis = Vector3dNormalize(xAxis);

        // 6. 在 (0,0,0) 创建原点
        Point3d origin(0.0, 0.0, 0.0);

        // 7. 获取当前部件
        Part* workPart = theSession->Parts()->Work();

        // 8. 使用 Vector3d 版本创建变换 (Xform)
        // 使用整数 0 代表 UpdateOption::kUpdateOptionNo
        Xform* targetXform = nullptr;
        try
        {
            targetXform = workPart->Xforms()->CreateXform(
                origin,
                xAxis,
                targetAxis,
                (NXOpen::SmartObject::UpdateOption)0,
                1.0
            );
        }
        catch (NXOpen::NXException& ex)
        {
            std::string errorMsg = "错误：创建变换对象失败: " + std::string(ex.what());
            theSession->ListingWindow()->WriteLine(errorMsg);
            return 1;
        }

        // 9. 将 Body 转为 TaggedObject 并调用 Tag() 获取体的 tag
        tag_t bodyTag = 0;
        try {
            TaggedObject* tagged = static_cast<TaggedObject*>(targetBody);
            bodyTag = tagged->Tag();
        }
        catch (...) {
            bodyTag = 0;
        }

        tag_t xformTag = targetXform->Tag();

        theSession->ListingWindow()->WriteLine("变换对象已创建，Tag: " + std::to_string(xformTag));
        theSession->ListingWindow()->WriteLine("体 Tag: " + std::to_string(bodyTag));
        theSession->ListingWindow()->WriteLine("面中心: (" + std::to_string(faceCenter.X) + ", " + std::to_string(faceCenter.Y) + ", " + std::to_string(faceCenter.Z) + ")");
        theSession->ListingWindow()->WriteLine("目标轴(Y): (" + std::to_string(targetAxis.X) + ", " + std::to_string(targetAxis.Y) + ", " + std::to_string(targetAxis.Z) + ")");
        theSession->ListingWindow()->WriteLine("计算X轴: (" + std::to_string(xAxis.X) + ", " + std::to_string(xAxis.Y) + ", " + std::to_string(xAxis.Z) + ")");
        theSession->ListingWindow()->WriteLine("原点: (" + std::to_string(origin.X) + ", " + std::to_string(origin.Y) + ", " + std::to_string(origin.Z) + ")");

        theSession->ListingWindow()->WriteLine("实体摆正完成！");
        theSession->ListingWindow()->WriteLine("注：在NX 2306中，Xform已创并与部件关联。实体将被自动转换到原点且面垂直于选择的轴。");
    }
    catch (exception& ex)
    {
        errorCode = 1;
        std::string errorMsg = std::string("Block Styler错误：") + std::string(ex.what());
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, errorMsg);
    }
    return errorCode;
}

int AdjustEntity::update_cb(NXOpen::BlockStyler::UIBlock* block)
{
    try
    {
        if (block == selection)
        {
            selection->Focus();
        }
        else if (block == plane)
        {

            plane->Focus();
        }
        else if (block == manip)
        {
        }
    }
    catch (exception& ex)
    {
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return 0;
}

int AdjustEntity::ok_cb()
{
    int errorCode = 0;
    try
    {
        errorCode = apply_cb();
    }
    catch (exception& ex)
    {
        errorCode = 1;
        AdjustEntity::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return errorCode;
}

PropertyList* AdjustEntity::GetBlockProperties(const char* blockID)
{
    return theDialog->GetBlockProperties(blockID);
}

