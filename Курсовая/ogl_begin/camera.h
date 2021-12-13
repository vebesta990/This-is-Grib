#define W 1000 // �������� ������� ����� mapW
#define H 1000 // �������� ������� ����� mapH 

// ��������� (������) ������
struct SCamera {
	float x, y, z;
	float Xrot, Zrot;
} camera;

// ������� ���������� ���������� ��������� ������
void Camera_Apply();
// ������� �������� ������
void Camera_Rotation(float xAngle, float zAngle);
// ������� �������� ������ ��������
void Camera_AutoMoveByMouse(int centerX, int centerY, float speed);
// ������� ����������� ������
void Camera_MoveDirection(int forwardMove, int rightMove, float speed);
// ��������� � ������ �� ���������� ����� SCP �� ������� (�������� ������� ����� �������)
BOOL IsCoordInMap(float x, float y);