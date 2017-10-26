#pragma once
#include "emacros.h"
#include "ecommon.h"

namespace e2d
{

class EApp;
class EObject;
class EScene;
class ENode;
class ETimer;
class EAction;
class EMouseListener;
class EKeyboardListener;

// ���������
class EObjectManager
{
	friend EApp;

public:
	// ��һ���ڵ�����ڴ��
	static void add(
		e2d::EObject * nptr
	);

	// ֪ͨ�ڴ��ˢ��
	static void notifyFlush();

private:
	// ˢ���ڴ��
	static void __flush();
};


// ��Ϣ������
class EMsgManager
{
	friend EApp;
	friend EScene;
	friend ENode;

public:
	// �������Ϣ������������
	static void bindListener(
		EMouseListener * listener,
		EScene * pParentScene,
		bool always = false /* �Ƿ�����Ϸ��ͣʱ��Ȼ���� */
	);

	// �������Ϣ���������ڵ�
	static void bindListener(
		EMouseListener * listener,
		ENode * pParentNode,
		bool always = false /* �Ƿ�����Ϸ��ͣʱ��Ȼ���� */
	);

	// ����������ͬ���Ƶ������Ϣ������
	static void startMouseListeners(
		const EString &name
	);

	// ֹͣ������ͬ���Ƶ������Ϣ������
	static void stopMouseListeners(
		const EString &name
	);

	// ɾ��������ͬ���Ƶ������Ϣ������
	static void delMouseListeners(
		const EString &name
	);

	// �������ڳ��������ӽڵ��ϵ����������Ϣ������
	static void startAllMouseListenersBindedWith(
		EScene * pParentScene
	);

	// ֹͣ���ڳ��������ӽڵ��ϵ����������Ϣ������
	static void stopAllMouseListenersBindedWith(
		EScene * pParentScene
	);

	// �������ڽڵ��ϵ����������Ϣ������
	static void startAllMouseListenersBindedWith(
		ENode * pParentNode
	);

	// ֹͣ���ڽڵ��ϵ����������Ϣ������
	static void stopAllMouseListenersBindedWith(
		ENode * pParentNode
	);

	// �������������Ϣ������
	static void startAllMouseListeners();

	// ֹͣ���������Ϣ������
	static void stopAllMouseListeners();

	// �󶨰�����Ϣ������������
	static void bindListener(
		EKeyboardListener * listener,
		EScene * pParentScene,
		bool always = false /* �Ƿ�����Ϸ��ͣʱ��Ȼ���� */
	);

	// �󶨰�����Ϣ���������ڵ�
	static void bindListener(
		EKeyboardListener * listener,
		ENode * pParentNode,
		bool always = false /* �Ƿ�����Ϸ��ͣʱ��Ȼ���� */
	);

	// ����������ͬ�İ�����Ϣ������
	static void startKeyboardListeners(
		const EString &name
	);

	// ֹͣ������ͬ�İ�����Ϣ������
	static void stopKeyboardListeners(
		const EString &name
	);

	// ɾ��������ͬ�İ�����Ϣ������
	static void delKeyboardListeners(
		const EString &name
	);

	// �������ڳ��������ӽڵ��ϵ����а�����Ϣ������
	static void startAllKeyboardListenersBindedWith(
		EScene * pParentScene
	);

	// ֹͣ���ڳ��������ӽڵ��ϵ����а�����Ϣ������
	static void stopAllKeyboardListenersBindedWith(
		EScene * pParentScene
	);

	// �������ڽڵ��ϵ����а�����Ϣ������
	static void startAllKeyboardListenersBindedWith(
		ENode * pParentNode
	);

	// ֹͣ���ڽڵ��ϵ����а�����Ϣ������
	static void stopAllKeyboardListenersBindedWith(
		ENode * pParentNode
	);

	// �������а�����Ϣ������
	static void startAllKeyboardListeners();

	// ֹͣ���а�����Ϣ������
	static void stopAllKeyboardListeners();

private:
	// ������м�����
	static void _clearManager();

	// ������ڽڵ��ϵ����������Ϣ������
	static void _clearAllMouseListenersBindedWith(
		ENode * pParentNode
	);

	// ������ڽڵ��ϵ����а�����Ϣ������
	static void _clearAllKeyboardListenersBindedWith(
		ENode * pParentNode
	);

	// �����Ϣ����
	static void MouseProc(
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	// ������Ϣ����
	static void KeyboardProc(
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);
};


// ��ʱ��������
class ETimerManager
{
	friend EApp;
	friend EScene;
	friend ENode;

public:
	// �󶨶�ʱ��������
	static void bindTimer(
		ETimer * timer,
		EScene * pParentScene
	);

	// �󶨶�ʱ�����ڵ�
	static void bindTimer(
		ETimer * timer,
		ENode * pParentNode
	);

	// ����������ͬ���ƵĶ�ʱ��
	static void startTimers(
		const EString &name
	);

	// ֹͣ������ͬ���ƵĶ�ʱ��
	static void stopTimers(
		const EString &name
	);

	// ɾ��������ͬ���ƵĶ�ʱ��
	static void delTimers(
		const EString &name
	);

	// �������ڳ��������ӽڵ��ϵ����ж�ʱ��
	static void startAllTimersBindedWith(
		EScene * pParentScene
	);

	// ֹͣ���ڳ��������ӽڵ��ϵ����ж�ʱ��
	static void stopAllTimersBindedWith(
		EScene * pParentScene
	);

	// �������ڽڵ��ϵ����ж�ʱ��
	static void startAllTimersBindedWith(
		ENode * pParentNode
	);

	// ֹͣ���ڽڵ��ϵ����ж�ʱ��
	static void stopAllTimersBindedWith(
		ENode * pParentNode
	);

	// �������ж�ʱ��
	static void startAllTimers();

	// ֹͣ���ж�ʱ��
	static void stopAllTimers();

private:
	// ��ն�ʱ��������
	static void _clearManager();

	// ��հ��ڽڵ��ϵ����ж�ʱ��
	static void _clearAllTimersBindedWith(
		ENode * pParentNode
	);

	// ���ö�ʱ��״̬
	static void _resetAllTimers();

	// ��ʱ��ִ�г���
	static void TimerProc();
};


// ����������
class EActionManager
{
	friend EApp;
	friend EScene;
	friend ENode;

public:
	// ���Ӷ���
	static void addAction(
		EAction * action
	);

	// �������ڽڵ��ϵ����ж���
	static void startAllActionsBindedWith(
		ENode * pTargetNode
	);

	// ��ͣ���ڽڵ��ϵ����ж���
	static void pauseAllActionsBindedWith(
		ENode * pTargetNode
	);

	// ֹͣ���ڽڵ��ϵ����ж���
	static void stopAllActionsBindedWith(
		ENode * pTargetNode
	);

	// �������ж���
	static void startAllActions();

	// ��ͣ���ж���
	static void pauseAllActions();

	// ֹͣ���ж���
	static void stopAllActions();

private:
	// ��ն���������
	static void _clearManager();

	// ��հ��ڽڵ��ϵ����ж���
	static void _clearAllActionsBindedWith(
		ENode * pTargetNode
	);

	// �������ж���״̬
	static void _resetAllActions();

	// ����ִ�г���
	static void ActionProc();
};


class EPhysicsManager
{
public:
	// ����״���ڵ�
	static void bindWith(
		EGeometry * geometry,
		ENode * node
	);

protected:
	// ��������ִ�г���
	static void PhysicsProc();
};

}