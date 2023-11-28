#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <setjmp.h>

#define SHARED_MEMORY_KEY 5522
#define MEMORY_SIZE 200

int signal_opponent = 1;
int mypid, enemypid;

int turn = 0;
char map[10][10]; //�� 

typedef struct { //�����޸𸮿� ������ ����ü 
	int act[30];
	int actCount;
	int shmid;
	int* Player;
}actInfo;

typedef struct {
	int score;
	int bp;
	int money;
	int posi; //���� ��ġ 
	int posj;
}player;

player p1, p2; //p2�� ���

void init_map(){
	for(int i=0; i<10; i++){
		for(int j=0; j<10; j++){
			map[i][j] = ' ';
		}
	}
}

//�ñ׳� �ޱ� �Լ�
void receive_signal()
{
	signal_opponent = 0;
	return;
}

//ó���� ȣ��Ʈ�� Ŭ���̾�Ʈ ��ġ����ŷ �� ����Ǵ� �ڵ�
int matchmaking() {
	FILE* fp;
	int iter;
	mypid = getpid();
	char buffer[256];

	printf("������ �õ��մϴ�.\n");

	fp = fopen("host.txt", "r");
	if (fp == NULL)
	{
		printf("ȣ��Ʈ ���� ����\n");

		fp = fopen("host.txt", "w");
		if (fp == NULL)
		{
			printf("host.txt ���� ���� ���� ���� ���� \n");
			exit(0);
		}
		fprintf(fp, "%d\n", mypid);
		fclose(fp);

		printf("Ŭ���̾�Ʈ�� ���� ��...\n");
		signal(SIGUSR1, receive_signal);

		while (signal_opponent)
		{
			sleep(1);
		}
		signal_opponent = 1;

		fp = fopen("client.txt", "r");
		if (fp == NULL)
		{
			printf("���� ���� ���� : client.txt ���� ����\n");
			exit(0);
		}
		fgets(buffer, 256, fp);
		enemypid = 0;
		for (iter = 0; buffer[iter] != '\n'; iter++)
		{
			enemypid *= 10;
			enemypid += (buffer[iter] - '0');
		}
		fclose(fp);
		printf("Ŭ���̾�Ʈ�� ���� ����\n");
		
		//printf("Ŭ���̾�Ʈ �� \nenemypid : %d\n", enemypid);
		//printf("mypid : %d\n", mypid);
		return 1;
	}
	else
	{
		printf("Ŭ���̾�Ʈ ���� ����\n");
		fgets(buffer, 256, fp);
		enemypid = 0;
		for (iter = 0; buffer[iter] != '\n'; iter++)
		{
			enemypid *= 10;
			enemypid += (buffer[iter] - '0');
		}
		fclose(fp);
		
		//printf("Ŭ���̾�Ʈ �� \nenemypid : %d\n", enemypid);
		//printf("mypid : %d\n", mypid);
		
		fp = fopen("client.txt", "w");
		if (fp == NULL)
		{
			printf("���� ���� ���� : client.txt ���� ����\n");
			exit(0);
		}
		fprintf(fp, "%d\n", mypid);
		fclose(fp);
		kill(enemypid, 10);
		printf("ȣ��Ʈ�� ���� ����\n");
		signal(SIGUSR1, receive_signal);
		return 2;
	}
	return 0;
}

//�ñ׳��� ������ �ñ׳��� �� ������ ���
void send_signal()
{
	kill(enemypid, 10);
	while (signal_opponent)
	{
		sleep(1);
	}
	signal_opponent = 1;
	return;
}

//ȣ��Ʈ �ʿ��� ���� ����� ȣ���ؾ� ��
void exit_host()
{
	system("rm -rf host.txt");
}

//Ŭ���̾�Ʈ �ʿ��� ���� ���� �� ȣ���ؾ� ��
void exit_client()
{
	system("rm -rf client.txt");
}

void display_clear(){
	system("clear");
}

void print_map() //�� ��� 
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			printf("%c", map[i][j]);
		}
		printf("\n");
	}
}

void print_playerPos() //�÷��̾� ��ġ ��� 
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (i == p1.posi && j == p1.posj)
				printf("i");
			else if (i == p2.posi && j == p2.posj)
				printf("o");
			else
				printf("%c", map[i][j]);
		}
		printf("\n");
	}
}

void p2_act(actInfo* info) //��� �ൿ ó��, �ڽ��� �ൿ�� �Է°� ���ÿ� ó�� 
{
	for (int i = 0; i < info->actCount; i++)
	{
		switch (info->Player[i])
		{
			//�����¿� �����ؼ� ó�� 
		case 1: //�� 
			p2.posi++;
			if (!(p2.posi == p1.posi && p2.posj == p1.posj)) //�ڽ��� �ִ� ���� �ƴϰ� ���� ���� ���̶�� �ڱ� ������ 
				if (!(map[p2.posi][p2.posj] == 'm' || map[p2.posi][p2.posj] == 'b' || map[p2.posi][p2.posj] == 'c' || map[p2.posi][p2.posj] == 's' || map[p2.posi][p2.posj] == 'a'
					|| map[p2.posi][p2.posj] == 'M' || map[p2.posi][p2.posj] == 'B' || map[p2.posi][p2.posj] == 'C' || map[p2.posi][p2.posj] == 'S' || map[p2.posi][p2.posj] == 'A'))
					map[p2.posi][p2.posj] = '#';

			break;
		case 2: //�� 
			p2.posi--;
			if (!(p2.posi == p1.posi && p2.posj == p1.posj))
				if (!(map[p2.posi][p2.posj] == 'm' || map[p2.posi][p2.posj] == 'b' || map[p2.posi][p2.posj] == 'c' || map[p2.posi][p2.posj] == 's' || map[p2.posi][p2.posj] == 'a'
					|| map[p2.posi][p2.posj] == 'M' || map[p2.posi][p2.posj] == 'B' || map[p2.posi][p2.posj] == 'C' || map[p2.posi][p2.posj] == 'S' || map[p2.posi][p2.posj] == 'A'))
					map[p2.posi][p2.posj] = '#';
			break;
		case 3: //��
			p2.posj++;
			if (!(p2.posi == p1.posi && p2.posj == p1.posj))
				if (!(map[p2.posi][p2.posj] == 'm' || map[p2.posi][p2.posj] == 'b' || map[p2.posi][p2.posj] == 'c' || map[p2.posi][p2.posj] == 's' || map[p2.posi][p2.posj] == 'a'
					|| map[p2.posi][p2.posj] == 'M' || map[p2.posi][p2.posj] == 'B' || map[p2.posi][p2.posj] == 'C' || map[p2.posi][p2.posj] == 'S' || map[p2.posi][p2.posj] == 'A'))
					map[p2.posi][p2.posj] = '#';
			break;
		case 4: //��
			p2.posj--;
			if (!(p2.posi == p1.posi && p2.posj == p1.posj))
				if (!(map[p2.posi][p2.posj] == 'm' || map[p2.posi][p2.posj] == 'b' || map[p2.posi][p2.posj] == 'c' || map[p2.posi][p2.posj] == 's' || map[p2.posi][p2.posj] == 'a'
					|| map[p2.posi][p2.posj] == 'M' || map[p2.posi][p2.posj] == 'B' || map[p2.posi][p2.posj] == 'C' || map[p2.posi][p2.posj] == 'S' || map[p2.posi][p2.posj] == 'A'))
					map[p2.posi][p2.posj] = '#';
			break;
		case 5: //�ӴϺ� ��ġ
			p2.money -= 3;
			map[p2.posi][p2.posj] = 'm';
			break;
		case 6: //�ൿ�� ��ġ 
			p2.money -= 4;
			map[p2.posi][p2.posj] = 'b';
			break;
		case 7: //������ ��ġ
			p2.money -= 2;
			map[p2.posi][p2.posj] = 'c';
			break;
		case 8: //���ھ�� ��ġ
			p2.money -= 2;
			map[p2.posi][p2.posj] = 's';
			break;
		case 9: //���ݺ� ��ġ
			p2.money -= 3;
			map[p2.posi][p2.posj] = 'a';
			break;

		}
	}
}

void scoreBlock(int pnum, int i, int j) //���ھ�� ó��, pnum�� 1�̸� �ڽ� 2�� ��� 
{
	int left = j - 1;
	int right = j + 1;
	int up = i - 1;
	int down = i + 1;

	int count = 1;

	if (pnum == 1) //�ڽ� 
	{
		if (left >= 0)
		{
			if (map[i][left] == '@' || map[i][left] == 'M' || map[i][left] == 'B' || map[i][left] == 'C' || map[i][left] == 'S' || map[i][left] == 'A')
				count++;
		}

		if (right <= 9)
		{
			if (map[i][right] == '@' || map[i][right] == 'M' || map[i][right] == 'B' || map[i][right] == 'C' || map[i][right] == 'S' || map[i][right] == 'A')
				count++;
		}

		if (up >= 0)
		{
			if (map[up][j] == '@' || map[up][j] == 'M' || map[up][j] == 'B' || map[up][j] == 'C' || map[up][j] == 'S' || map[up][j] == 'A')
				count++;
		}

		if (down <= 9)
		{
			if (map[down][j] == '@' || map[down][j] == 'M' || map[down][j] == 'B' || map[down][j] == 'C' || map[down][j] == 'S' || map[down][j] == 'A')
				count++;
		}
		p1.score += count;
	}

	if (pnum == 2) //��� 
	{
		if (left >= 0)
		{
			if (map[i][left] == '#' || map[i][left] == 'm' || map[i][left] == 'b' || map[i][left] == 'c' || map[i][left] == 's' || map[i][left] == 'a')
				count++;
		}

		if (right <= 9)
		{
			if (map[i][right] == '#' || map[i][right] == 'm' || map[i][right] == 'b' || map[i][right] == 'c' || map[i][right] == 's' || map[i][right] == 'a')
				count++;
		}

		if (up >= 0)
		{
			if (map[up][j] == '#' || map[up][j] == 'm' || map[up][j] == 'b' || map[up][j] == 'c' || map[up][j] == 's' || map[up][j] == 'a')
				count++;
		}

		if (down <= 9)
		{
			if (map[down][j] == '#' || map[down][j] == 'm' || map[down][j] == 'b' || map[down][j] == 'c' || map[down][j] == 's' || map[down][j] == 'a')
				count++;
		}
		p2.score += count;
	}

}

void attackBlock(int pnum, int i, int j) //���ݺ� ó��, pnum�� 1�̸� �ڽ� 2�� ���
{
	int left = j - 1;
	int right = j + 1;
	int up = i - 1;
	int down = i + 1;

	int count = 0;

	if (pnum == 1) //�ڽ� 
	{
		if (left >= 0)
		{
			if (map[i][left] == '#' || map[i][left] == 'm' || map[i][left] == 'b' || map[i][left] == 'c' || map[i][left] == 's' || map[i][left] == 'a')
				count++;
		}

		if (right <= 9)
		{
			if (map[i][right] == '#' || map[i][right] == 'm' || map[i][right] == 'b' || map[i][right] == 'c' || map[i][right] == 's' || map[i][right] == 'a')
				count++;
		}

		if (up >= 0)
		{
			if (map[up][j] == '#' || map[up][j] == 'm' || map[up][j] == 'b' || map[up][j] == 'c' || map[up][j] == 's' || map[up][j] == 'a')
				count++;
		}

		if (down <= 9)
		{
			if (map[down][j] == '#' || map[down][j] == 'm' || map[down][j] == 'b' || map[down][j] == 'c' || map[down][j] == 's' || map[down][j] == 'a')
				count++;
		}
		p2.score -= count;
	}

	if (pnum == 2) //��� 
	{
		if (left >= 0)
		{
			if (map[i][left] == '@' || map[i][left] == 'M' || map[i][left] == 'B' || map[i][left] == 'C' || map[i][left] == 'S' || map[i][left] == 'A')
				count++;
		}

		if (right <= 9)
		{
			if (map[i][right] == '@' || map[i][right] == 'M' || map[i][right] == 'B' || map[i][right] == 'C' || map[i][right] == 'S' || map[i][right] == 'A')
				count++;
		}

		if (up >= 0)
		{
			if (map[up][j] == '@' || map[up][j] == 'M' || map[up][j] == 'B' || map[up][j] == 'C' || map[up][j] == 'S' || map[up][j] == 'A')
				count++;
		}

		if (down <= 9)
		{
			if (map[down][j] == '@' || map[down][j] == 'M' || map[down][j] == 'B' || map[down][j] == 'C' || map[down][j] == 'S' || map[down][j] == 'A')
				count++;
		}
		p1.score -= count;
	}

}


void update() //�� ���� ���� ������ ȣ�� 
{
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
		{
			switch (map[i][j])
			{
			case 'M': //�ڽ�  
				p1.money++;
				break;
			case 'B':
				p1.bp++;
				break;
			case 'S':
				scoreBlock(1, i, j);
				break;
			case 'A':
				attackBlock(1, i, j);
				break;
			case 'm': //��� 
				p2.money++;
				break;
			case 'b':
				p2.bp++;
				break;
			case 's':
				scoreBlock(2, i, j);
				break;
			case 'a':
				attackBlock(2, i, j);
				break;

			}
		}
}

void initMemory(actInfo* info, int n)
{
	if(n == 1){
		info->shmid = shmget((key_t)SHARED_MEMORY_KEY, (size_t)MEMORY_SIZE, 0666 | IPC_CREAT);
		if (info->shmid == -1) {
			printf("shmget failed\n");
			exit(0);
		}
	}

	if(n == 2){
		info->shmid = shmget((key_t)SHARED_MEMORY_KEY, (size_t)MEMORY_SIZE, 0);
		if (info->shmid == -1) {
			printf("shmget failed\n");
			exit(0);
		}
	}

	// ���� �޸𸮿� ���� ���� ����
	info->Player = (int*)shmat(info->shmid, NULL, 0);
	if (info->Player == (int*)-1) {
		printf("shmat failed\n");
		exit(0);
	}

	//�޸� �ʱ�ȭ
	memset(info->Player, 0, MEMORY_SIZE);
}

void print(actInfo* info) 
{
	printf("\n");
	info->actCount = 6;
	for (int i = 0; i < info->actCount; i++) { //�����޸𸮿� ����� �ൿ������ ���
		printf("act %d : %d", i, info->Player[i]);

		if (info->Player[i] == 0)
		{
			printf("-> �ƹ��ൿ�� ���� �ʾҽ��ϴ�.\n");
		}

		if (info->Player[i] == 1)
		{
			printf("-> ���� �̵��Ͽ����ϴ�.\n");
		}

		if (info->Player[i] == 2)
		{
			printf("-> �Ʒ��� �̵��Ͽ����ϴ�.\n");
		}

		if (info->Player[i] == 3)
		{
			printf("-> �������� �̵��Ͽ����ϴ�.\n");
		}

		if (info->Player[i] == 4)
		{
			printf("-> ���������� �̵��Ͽ����ϴ�.\n");
		}

		if (info->Player[i] == 5)
		{
			printf("-> �ӴϺ���� ��ġ�߽��ϴ�.\n");
		}

		if (info->Player[i] == 6)
		{
			printf("-> �ൿ����� ��ġ�߽��ϴ�.\n");
		}

		if (info->Player[i] == 7)
		{
			printf("-> ��������� ��ġ�߽��ϴ�.\n");
		}

		if (info->Player[i] == 8)
		{
			printf("-> ���ھ����� ��ġ�߽��ϴ�.\n");
		}

		if (info->Player[i] == 9)
		{
			printf("-> ���ݺ���� ��ġ�߽��ϴ�.\n");
		}
	}
	p2_act(info); //�� ������Ʈ
	print_map();  //�� ���

	for (int i = 0; i < info->actCount; i++) { //�����޸𸮿��� �÷��̾��� �ൿ������ �ʱ�ȭ
		info->Player[i] = 0;
	}
}

void SaveInfo(actInfo* info) {

	//���� �޸𸮿� �ൿ���� ����
	for (int i = 0; i < info->actCount; i++) {
		info->Player[i] = info->act[i];
	}

	//�ڽ��� �ൿ ���� ���
	for (int i = 0; i < info->actCount; i++) {
		printf("Player act = ");
		printf("%d\n", info->Player[i]);
	}
}

void actInput(actInfo* info)
{
	int in;
	print_map();
	while (p1.bp > 0)
	{
		printf("\n");
		printf("������ �Ͻðڽ��ϱ�?\n���� : %d, �� : %d, �ൿ����Ʈ : %d\n��� ���� : %d\n", p1.score, p1.money, p1.bp, p2.score);
		printf("1. �̵� 2. ����ġ 3. �� ��� 4. �÷��̾� ��ġ ���: ");
		scanf("%d", &in);

		if (in == 1)
		{
			printf("1. �� 2. �� 3. �� 4. �� (back : �� ���� Ű) : ");
			scanf("%d", &in);
			if (in == 1)
			{
				if (p1.posi <= 0)
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				if (map[p1.posi - 1][p1.posj] == 'c')
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}

				p1.posi -= 1;
				p1.bp -= 1;
				if (!(p2.posi == p1.posi && p2.posj == p1.posj)) //�ڽ��� �ִ� ���� �ƴϰ� ���� ���� ���̶�� �ڱ� ������ 
					if (!(map[p1.posi][p1.posj] == 'm' || map[p1.posi][p1.posj] == 'b' || map[p1.posi][p1.posj] == 'c' || map[p1.posi][p1.posj] == 's' || map[p1.posi][p1.posj] == 'a'
						|| map[p1.posi][p1.posj] == 'M' || map[p1.posi][p1.posj] == 'B' || map[p1.posi][p1.posj] == 'C' || map[p1.posi][p1.posj] == 'S' || map[p1.posi][p1.posj] == 'A'))
						map[p1.posi][p1.posj] = '@';
				info->act[info->actCount] = 1;
				info->actCount++;
				print_playerPos();
			}
			else if (in == 2)
			{
				if (p1.posi >= 9)
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				if (map[p1.posi + 1][p1.posj] == 'c')
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				p1.posi += 1;
				p1.bp -= 1;
				if (!(p2.posi == p1.posi && p2.posj == p1.posj)) //�ڽ��� �ִ� ���� �ƴϰ� ���� ���� ���̶�� �ڱ� ������ 
					if (!(map[p1.posi][p1.posj] == 'm' || map[p1.posi][p1.posj] == 'b' || map[p1.posi][p1.posj] == 'c' || map[p1.posi][p1.posj] == 's' || map[p1.posi][p1.posj] == 'a'
						|| map[p1.posi][p1.posj] == 'M' || map[p1.posi][p1.posj] == 'B' || map[p1.posi][p1.posj] == 'C' || map[p1.posi][p1.posj] == 'S' || map[p1.posi][p1.posj] == 'A'))
						map[p1.posi][p1.posj] = '@';
				info->act[info->actCount] = 2;
				info->actCount++;
				print_playerPos();
			}
			else if (in == 3)
			{
				if (p1.posj <= 0)
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				if (map[p1.posi][p1.posj - 1] == 'c')
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				p1.posj -= 1;
				p1.bp -= 1;
				if (!(p2.posi == p1.posi && p2.posj == p1.posj)) //�ڽ��� �ִ� ���� �ƴϰ� ���� ���� ���̶�� �ڱ� ������ 
					if (!(map[p1.posi][p1.posj] == 'm' || map[p1.posi][p1.posj] == 'b' || map[p1.posi][p1.posj] == 'c' || map[p1.posi][p1.posj] == 's' || map[p1.posi][p1.posj] == 'a'
						|| map[p1.posi][p1.posj] == 'M' || map[p1.posi][p1.posj] == 'B' || map[p1.posi][p1.posj] == 'C' || map[p1.posi][p1.posj] == 'S' || map[p1.posi][p1.posj] == 'A'))
						map[p1.posi][p1.posj] = '@';
				info->act[info->actCount] = 3;
				info->actCount++;
				print_playerPos();
			}
			else if (in == 4)
			{
				if (p1.posj >= 9)
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				if (map[p1.posi][p1.posj + 1] == 'c')
				{
					printf("�̵��� �� �����ϴ�\n");
					continue;
				}
				p1.posj += 1;
				p1.bp -= 1;
				if (!(p2.posi == p1.posi && p2.posj == p1.posj)) //�ڽ��� �ִ� ���� �ƴϰ� ���� ���� ���̶�� �ڱ� ������ 
					if (!(map[p1.posi][p1.posj] == 'm' || map[p1.posi][p1.posj] == 'b' || map[p1.posi][p1.posj] == 'c' || map[p1.posi][p1.posj] == 's' || map[p1.posi][p1.posj] == 'a'
						|| map[p1.posi][p1.posj] == 'M' || map[p1.posi][p1.posj] == 'B' || map[p1.posi][p1.posj] == 'C' || map[p1.posi][p1.posj] == 'S' || map[p1.posi][p1.posj] == 'A'))
						map[p1.posi][p1.posj] = '@';
				info->act[info->actCount] = 4;
				info->actCount++;
				print_playerPos();
			}
			else
				continue;
		}

		else if (in == 2)
		{
			if (map[p1.posi][p1.posj] == 'M' || map[p1.posi][p1.posj] == 'B' || map[p1.posi][p1.posj] == 'C' || map[p1.posi][p1.posj] == 'S' || map[p1.posi][p1.posj] == 'A'
				|| map[p1.posi][p1.posj] == 'm' || map[p1.posi][p1.posj] == 'b' || map[p1.posi][p1.posj] == 'c' || map[p1.posi][p1.posj] == 's' || map[p1.posi][p1.posj] == 'a')
			{
				printf("�̹� ��ġ�� ���� �ֽ��ϴ�\n");
				continue;
			}

			printf("1. �ӴϺ�(3��) 2. �ൿ��(4��) 3. ������(2��) 4. ���ھ��(2��) 5.���ݺ�(3��) (back : �� ���� Ű) : ");
			scanf("%d", &in);
			switch (in)
			{
			case 1:
				if (p1.money >= 3)
				{
					p1.money -= 3;
					map[p1.posi][p1.posj] = 'M';
					info->act[info->actCount] = 5;
					info->actCount++;
				}
				else
					printf("���� �����մϴ�\n");
				break;
			case 2:
				if (p1.money >= 4)
				{
					p1.money -= 4;
					map[p1.posi][p1.posj] = 'B';
					info->act[info->actCount] = 6;
					info->actCount++;
				}
				else
					printf("���� �����մϴ�\n");
				break;
			case 3:
				if (p1.money >= 2)
				{
					p1.money -= 2;
					map[p1.posi][p1.posj] = 'C';
					info->act[info->actCount] = 7;
					info->actCount++;
				}
				else
					printf("���� �����մϴ�\n");
				break;
			case 4:
				if (p1.money >= 2)
				{
					p1.money -= 2;
					map[p1.posi][p1.posj] = 'S';
					info->act[info->actCount] = 8;
					info->actCount++;
				}
				else
					printf("���� �����մϴ�\n");
				break;
			case 5:
				if (p1.money >= 3)
				{
					p1.money -= 3;
					map[p1.posi][p1.posj] = 'A';
					info->act[info->actCount] = 9;
					info->actCount++;
				}
				else
					printf("���� �����մϴ�\n");
				break;
			}

		}
		else if (in == 3)
		{

			print_map();
		}
		else if (in == 4)
		{

			print_playerPos();
		}
	}
}

int main() {
	int n=0;
	
	p1.score = 0; //�÷��̾� �ʱ�ȭ 
	p1.bp = 3;
	p1.money = 10;
	p1.posi = 9;
	p1.posj = 9;

	p2.score = 0;
	p2.bp = 3;
	p2.money = 10;
	p2.posi = 0;
	p2.posj = 0;
	
	init_map();
	
	map[0][0] = '#';
	map[9][9] = '@';

	actInfo info;
	
	n = matchmaking();
	initMemory(&info, n);
	//print_map();
	
	//ȣ��Ʈ(����)
	if(n==1){
		while (1)
		{
			info.actCount = 0;
			actInput(&info);
			SaveInfo(&info); //���� �޸𸮿� �ൿ ����
			send_signal(); //�ñ׳��� ���� �� �ñ׳��� �� ������ ��ٸ� 
			print(&info);//�����޸𸮿��� ��� �ൿ �о�ͼ� p2_act ȣ��(��� �ൿ ó��)
			turn++;
			update();
			if (turn > 15)
			{
				if (p1.score > p2.score)
					printf("�¸�");
				else if (p1.score < p2.score)
					printf("�й�");
				else
					printf("���º�");

				break;
			}
			p1.money += 2;
			p2.money += 2;

			p1.bp += 2;
			p2.bp += 2;
		}
	}
	if(n==2){
		while (1)
		{
			if(turn == 0){
				while (signal_opponent){
					sleep(1);
				}
				signal_opponent = 1;
			}
			print(&info);//�����޸𸮿��� ��� �ൿ �о�ͼ� p2_act ȣ��(��� �ൿ ó��)
			info.actCount = 0;
			actInput(&info);
			SaveInfo(&info); //���� �޸𸮿� �ൿ ����
			turn++;
			update();
			send_signal(); //�ñ׳��� ���� �� �ñ׳��� �� ������ ��ٸ� 
			if (turn > 15)
			{
				if (p1.score > p2.score)
					printf("�¸�");
				else if (p1.score < p2.score)
					printf("�й�");
				else
					printf("���º�");

				break;
			}
			p1.money += 2;
			p2.money += 2;

			p1.bp += 2;
			p2.bp += 2;
		}
	}

	//���� �޸� ����
	if (shmdt(info.Player) == -1) {
		perror("shmdt");
		exit(1);
	}

	// ���� �޸� ����
	if (shmctl(info.shmid, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		exit(1);
	}
	
	if(n==1){
		exit_host();
	}
	
	if(n==2){
		exit_client();
	}

	return 0;
}
