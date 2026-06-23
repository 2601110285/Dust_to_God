#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <conio.h> // 방향키 입력을 즉각 받기(_kbhit)위함
#include <windows.h> // 글자 색을 바꾸거나 커서 위치를 조작

// 맵 크기 고정
#define ROWS        20 
#define COLS        40

// 목표 질량 설정
#define BASE_MASS_STAGE2  10 
#define BASE_MASS_STAGE3  30 
#define BASE_MASS_STAGE4  60 
#define BASE_MASS_ENDING  100 

// 게임 단계
#define STAGE_1      1  // 숫자 1 = 먼지
#define STAGE_2      2  // 숫자 2 = 원시별
#define STAGE_3      3  // 숫자 3 = 원시은하
#define STAGE_4      4  // 숫자 4 = 나선은하
#define STAGE_ENDING 5 // 숫자 5 = 엔딩
#define MINIGAME_1   91 // 숫자 91 = 미니게임

// 방향키
#define KEY_UP      72 // 키보드 위쪽 화살표가 눌렸을 때 컴퓨터가 인식하는 번호
#define KEY_DOWN    80 // 키보드 아래쪽 화살표 ｀｀｀
#define KEY_LEFT    75 // 키보드 왼쪽 화살표 ｀｀｀
#define KEY_RIGHT   77 // 키보드 오른쪽 화살표 ｀｀｀
#define KEY_SPECIAL 224 // 방향키를 누를 때 컴퓨터가 먼저 보내는 특수 신호 번호
#define KEY_ESC     27 // 게임 종료를 위한 ESC 키의 인식 번호

// 전역 변수
char map[ROWS][COLS]; // 게임 배경인 20x40 크기의 공간 데이터를 담을 2차원 배열
int playerRow, playerCol, playerMass; // 내 캐릭터의 세로, 가로 위치와 현재 질량을 기억
int bhRow, bhCol; // 블랙홀 좌표 // 블랙홀의 세로, 가로 위치를 기억
int currentStage, isGameOver, turnCount; // 현재 몇 단계인지, 죽었는지, 시간이 얼마나 흘렀는지 저장
int difficultyMult, targetMass; // 내가 고른 난이도 수치와 이번 단계의 목표 점수를 저장

// 함수 선언
void setCursorPosition(int x, int y); // 화면의 특정 좌표로 커서를 옮기는 함수가 밑에 있다고 알림
void hideCursor(); // 화면에서 커서를 숨기는 함수
void setColor(int colorCode); // 글자 색깔을 바꾸는 함수

int  showMainMenu();   // 시작 메뉴를 띄우고 사용자의 선택을 숫자로 반환하는 함수
void showHelp();       // 게임 방법을 설명하는 화면을 띄우는 함수
int  askRestart();     // 게임이 끝난 후 다시 할지 묻고 결과를 반환하는 함수

void selectDifficulty(); // 난이도를 입력받는 함수
void initMap(int stage); // 새로운 단계에 맞게 맵과 블랙홀 위치를 초기 세팅하는 함수
void drawMap(); // 배열에 담긴 데이터를 화면에 그리는 함수
void drawHUD(); // 화면 위에 내 점수와 상태를 보여주는 함수
void processInput(); // 키보드 입력을 받아 내 캐릭터를 움직이는 함수
void spawnContinuousItems(); // 시간이 지날 때마다 빈 공간에 먹이를 뿌리는 함수
void updateBlackHoleChase(); // 블랙홀이 내 캐릭터를 쫓아오게 위치를 조작하는 함수
void runMiniGame1(); // 미니게임을 실행하는 함수
void playEndingAnimation(); // 클리어 시 폭발하는 이펙트를 그리는 함수
void drawSupermassiveGalaxy(); // 엔딩에서 은하수 그림을 그리는 함수
void showGameOver(); // 죽었을 때 게임 오버 화면을 띄우는 함수

/* ============================================================
 * [시스템 제어 및 컬러 함수]
 * ============================================================ */
void setCursorPosition(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y }; 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
} 

void hideCursor() { 
    CONSOLE_CURSOR_INFO ci = { 100, FALSE }; 
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci); // 커서 숨김
} 

void setColor(int code) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), code); // 윈도우 명령어를 통해 콘솔 텍스트 색상을 입력한 번호로 바꿈
} 

/* ============================================================
 * [메인 메뉴 및 설명 화면]
 * ============================================================ */
int showMainMenu() { 
    int key; 
    while (1) { 
        system("cls"); 
        setColor(11); 
        printf("\n\n\t====================================\n");
        printf("\t=                                  =\n"); 
        printf("\t=        PROJECT: OVERFLOW         =\n"); 
        printf("\t=                                  =\n"); 
        printf("\t====================================\n\n"); 

        setColor(15);
        printf("\t          [1] 게임 시작\n"); 
        printf("\t          [2] 게임 설명\n"); 
        printf("\t          [3] 게임 종료\n"); 

        setColor(7); 
        printf("\n\t   원하는 메뉴의 숫자 키를 누르세요."); 

        while (1) {
            if (_kbhit()) { 
                key = _getch(); 
                if (key == '1') return 1; 
                if (key == '2') { 
                    showHelp();           
                    break;               
                } 
                if (key == '3') return 0; 
            } 
        } 
    } 
} 

void showHelp() { 
    system("cls"); 
    setColor(14); 
    printf("\n\n\t       [ 게 임  설 명 ]\n\n"); 
    setColor(7); 
    printf("\t 1. 방향키(↑↓←→)로 캐릭터(P)를 움직입니다.\n"); 
    printf("\t    맵에 뿌려진 별들을 흡수하여 질량을 키우세요.\n\n");

    setColor(12); 
    printf("\t 2. 2단계부터는 당신의 질량을 노리는\n"); 
    printf("\t    거대한 블랙홀 '@'가 쫓아옵니다.\n"); 
    printf("\t    잡히면 질량을 뺏기며, 0이 되면 파멸합니다.\n\n");

    setColor(11);
    printf("\t 3. 블랙홀을 피해 무한한 질량의 임계점에 도달하면\n");
    printf("\t    새로운 우주를 창조할 수 있습니다.\n\n");

    setColor(8); 
    printf("\t 아무 키나 누르면 메뉴로 돌아갑니다...");
    _getch();
} 

// 난이도 선택
void selectDifficulty() { 
    system("cls"); 
    setColor(11); 
    printf("\n\n\t====================================\n"); 
    printf("\t=        난 이 도  선 택           =\n"); 
    printf("\t====================================\n\n"); 
    setColor(10); printf("\t          [1] 하 (빠른 성장)\n"); 
    setColor(14); printf("\t          [2] 중 (일반 모드)\n"); 
    setColor(12); printf("\t          [3] 상 (포식자 가속)\n"); 

    setColor(7); 
    printf("\n\t   원하는 난이도의 숫자 키를 누르세요."); 

    int key = 0;
    while (key < '1' || key > '3')
        if (_kbhit()) key = _getch(); 
    difficultyMult = key - '0'; 
    targetMass = BASE_MASS_STAGE2 * difficultyMult; // 목표 점수를 (기본값 * 난이도 배수)로 계산해 세팅

    system("cls"); 
} 

int askRestart() { 
    setColor(14); 
    printf("\n\t 다시 플레이하시겠습니까? (R: 재시작 / ESC: 종료)\n");
    setColor(7); 

    while (1) { // 올바른 키를 누를 때까지 대기
        if (_kbhit()) { 
            int key = _getch(); // 무슨 키를 눌렀는지 가져옴
            if (key == 'r' || key == 'R') return 1; 
            if (key == KEY_ESC) return 0;
        } 
    } 
} 

/* ============================================================
 * [초기화 및 로직 함수들]
 * ============================================================ */
// 스테이지
void initMap(int stage) { 
    int i, j; 
    for (i = 0; i < ROWS; i++) { 
        for (j = 0; j < COLS; j++) map[i][j] = ' '; 
    } 
    playerRow = ROWS / 2; playerCol = COLS / 2; 

    bhRow = -1; bhCol = -1; 
    currentStage = stage; 
    turnCount = 0; 

    if (stage == STAGE_1) { 
        targetMass = BASE_MASS_STAGE2 * difficultyMult; 
        for (i = 0; i < 80; i++) map[rand() % ROWS][rand() % COLS] = ',';
    } 
    else if (stage == STAGE_2) { 
        bhRow = 1; bhCol = 1;
        targetMass = BASE_MASS_STAGE3 * difficultyMult;
        for (i = 0; i < 40; i++) map[rand() % ROWS][rand() % COLS] = '*';
    } 
    else if (stage == STAGE_3) {
        bhRow = 1; bhCol = 1; 
        targetMass = BASE_MASS_STAGE4 * difficultyMult; 
        for (i = 0; i < 15; i++) map[rand() % ROWS][rand() % COLS] = 'G';
    } 
    else if (stage == STAGE_4) {
        bhRow = 1; bhCol = 1;
        targetMass = BASE_MASS_ENDING * difficultyMult;
        for (i = 0; i < 15; i++) map[rand() % ROWS][rand() % COLS] = 'S';
    } 
} 

// 아이템 추가 생성
void spawnContinuousItems() { 
    if (currentStage == MINIGAME_1) return; // 미니게임 중일 때는 함수 종료

    int r = rand() % ROWS; 
    int c = rand() % COLS; 
    if (map[r][c] == ' ' && !(r == playerRow && c == playerCol)) { 
        if (currentStage == STAGE_1 && turnCount % 3 == 0) map[r][c] = ','; 
        else if (currentStage == STAGE_2 && turnCount % 6 == 0) map[r][c] = '*'; 
        else if (currentStage == STAGE_3 && turnCount % 8 == 0) map[r][c] = 'G'; 
        else if (currentStage == STAGE_4 && turnCount % 10 == 0) map[r][c] = 'S'; 
    } 
} 

// 블랙홀 추격
void updateBlackHoleChase() { 
    if (currentStage < STAGE_2 || currentStage > STAGE_4) return; // 1단계거나 미니게임 중 블랙홀이 안 나오므로 바로 종료

    int moveDelay = 5 - difficultyMult;
    if (turnCount % moveDelay == 0) { 
        if (bhRow < playerRow) bhRow++; else if (bhRow > playerRow) bhRow--; // 블랙홀이 플레이어보다 위에 있으면 아래로, 아래에 있으면 위로 1칸
        if (bhCol < playerCol) bhCol++; else if (bhCol > playerCol) bhCol--; // 블랙홀이 플레이어보다 왼쪽에 있으면 오른쪽으로, 오른쪽에 있으면 왼쪽으로 1칸
    } 

    int dist = abs(bhRow - playerRow) + abs(bhCol - playerCol);
    if (dist <= 4 && turnCount % 2 == 0) { 
        if (playerRow < bhRow) playerRow++; else if (playerRow > bhRow) playerRow--; // 세로 방향으로 블랙홀 쪽으로 1칸
        if (playerCol < bhCol) playerCol++; else if (playerCol > bhCol) playerCol--; // 가로 방향으로 블랙홀 쪽으로 1칸
    } 

    if (dist <= 1) { // 블랙홀에 끌림
        playerMass -= (difficultyMult * 2); //  난이도에 비례해 질량 -
        if (playerMass <= 0) isGameOver = 1;
    }
} 

// 렌더링 최적화
void drawMap() { 
    setCursorPosition(0, 0);
    setColor(8); 
    for (int j = 0; j < COLS + 2; j++) printf("-");
    printf("\n"); 

    for (int i = 0; i < ROWS; i++) { 
        setColor(8); printf("|"); 
        for (int j = 0; j < COLS; j++) { 
            if (i == playerRow && j == playerCol) { 
                if (currentStage == MINIGAME_1) { setColor(14); printf("^"); }
                else { setColor(11); printf("P"); } 
            } 
            else if (i == bhRow && j == bhCol && currentStage >= STAGE_2 && currentStage <= STAGE_4) { 
                setColor(13); printf("@"); 
            } 
            else { 
                char c = map[i][j];
                if (c == ',') setColor(8); 
                else if (c == '*') setColor(14); 
                else if (c == 'G') setColor(5); 
                else if (c == 'S') setColor(7); 
                else if (c == '#') setColor(12); 
                printf("%c", c); 
            } 
        } 
        setColor(8); printf("|\n"); 
    } 
    for (int j = 0; j < COLS + 2; j++) printf("-");
    printf("\n");
    setColor(7);
} 

// 현재 상태 출력
void drawHUD() { 
    setColor(11); printf("\n === 우주 스케일: ");
    setColor(15);
    if (currentStage == STAGE_1) printf("우주 먼지"); 
    else if (currentStage == STAGE_2) printf("원시별"); 
    else if (currentStage == STAGE_3) printf("원시 은하"); 
    else if (currentStage == STAGE_4) printf("나선 은하"); 

    setColor(11); printf(" ===\n");
    setColor(7); printf(" 질량: %d / %d  |  ", playerMass, targetMass);
    if (currentStage >= STAGE_2 && currentStage <= STAGE_4) 
        setColor(12); printf("블랙홀과의 거리: %d", abs(bhRow - playerRow) + abs(bhCol - playerCol)); 
    printf("\n"); 
} 

// 플레이어 이동 및 아이템 상호작용
void processInput() {
    if (_kbhit()) { 
        int key = _getch();
        if (key == KEY_SPECIAL) { // 특수키 들어옴?
            key = _getch(); // 진짜 화살표 방향 값을 두 번째로 꺼내옴
            if (key == KEY_UP && playerRow > 0) playerRow--; // 위 방향키, 맨 위 벽이 아니라면 Y좌표를 1 빼서 위
            if (key == KEY_DOWN && playerRow < ROWS - 1) playerRow++; // 아래 방향키, 맨 아래 벽이 아니라면 Y좌표를 1 더해 아래
            if (key == KEY_LEFT && playerCol > 0) playerCol--; // 왼쪽 방향키, 왼쪽 끝 벽이 아니라면 X좌표를 1 빼서 왼쪽
            if (key == KEY_RIGHT && playerCol < COLS - 1) playerCol++; // 오른쪽 방향키, 오른쪽 끝 벽이 아니라면 X좌표를 1 더해 오른쪽
        } 

        if (currentStage != MINIGAME_1) { 
            char item = map[playerRow][playerCol];
            if (item == ',') { playerMass += 1; map[playerRow][playerCol] = ' '; } 
            else if (item == '*') { playerMass += 2; map[playerRow][playerCol] = ' '; } 
            else if (item == 'G') { playerMass += 5; map[playerRow][playerCol] = ' '; } 
            else if (item == 'S') { playerMass += 10; map[playerRow][playerCol] = ' '; } 
        } 
    } 
} 

/* ============================================================
 * [미니게임: 운석 지대 통과]
 * ============================================================ */
void runMiniGame1() { 
    system("cls"); setColor(12); 
    printf("\n\n\t[ 미니게임: 거대 운석 지대 돌파 ]\n");
    setColor(7);
    printf("\t블랙홀을 따돌리기 위해 운석 지대를 통과해야 합니다.\n"); 
    printf("\t경고: 운석('#')에 한 번이라도 스치면 즉시 파괴됩니다!\n"); 
    Sleep(3000); 

    system("cls");

    bhRow = -1; bhCol = -1; // 미니게임 중에는 블랙홀x

    currentStage = MINIGAME_1; 
    for (int i = 0; i < ROWS; i++) 
        for (int j = 0; j < COLS; j++) map[i][j] = ' '; 
  
    playerRow = ROWS - 2; 
    playerCol = COLS / 2; 

    int miniTurn = 0; 
    int maxTurns = 120; 
    int dropDelay = 4 - difficultyMult; // 난이도가 높을수록 떨어지는 속도가 짧아져 운석이 자주 떨어짐

    while (miniTurn < maxTurns && !isGameOver) { 
        processInput(); 

        if (miniTurn % dropDelay == 0) { 
            for (int i = ROWS - 1; i > 0; i--) { 
                for (int j = 0; j < COLS; j++) 
                    map[i][j] = map[i - 1][j]; // 윗줄에 있던 운석을 아래쪽 칸으로 복사
            }
            for (int j = 0; j < COLS; j++) map[0][j] = ' '; 
            int meteorCount = difficultyMult * 2; // 새로 생성할 운석 개수 난이도 비례
            for (int k = 0; k < meteorCount; k++)
                map[0][rand() % COLS] = '#'; 
        } 

        if (map[playerRow][playerCol] == '#')
            isGameOver = 1; 

        drawMap();
        setColor(14); printf("\n=== [ 미니게임: 운석 지대 돌파 ] ===\n"); 
        setColor(7);  printf("남은 돌파 거리: %d / %d\n", maxTurns - miniTurn, maxTurns); 
        setColor(12); printf("경고: 운석에 닿으면 즉시 파멸합니다!\n");

        miniTurn++;
        Sleep(40);
    }

    if (!isGameOver) { 
        system("cls"); setColor(10); 
        printf("\n\t운석 지대를 무사히 돌파했습니다!\n");
        Sleep(2000); 
        initMap(STAGE_4); 
    } 
} 

/* ============================================================
 * [엔딩: 빅뱅 연출 및 나선 은하 렌더링]
 * ============================================================ */
void playEndingAnimation() {
    int r, i, j; 
    int centerRow = ROWS / 2; 
    int centerCol = COLS / 2; 

    system("cls"); 

    system("color F0"); // 윈도우 배경색 전체를 하얗게
    Sleep(80); 
    system("color 0F"); // 다시 까만 배경
    Sleep(50); 

    for (r = 1; r <= 35; r += 2) { 
        setCursorPosition(0, 0); 
        for (i = 0; i < ROWS; i++) { 
            for (j = 0; j < COLS; j++) { 
                int dist = (abs(i - centerRow) * 2) + abs(j - centerCol);

                if (dist == r || dist == r + 1) { setColor(15); printf("*"); } 
                else if (dist == r - 1 || dist == r - 2) { setColor(14); printf("O"); } 
                else if (dist == r - 3 || dist == r - 4) { setColor(12); printf("."); } 
                else { 
                    if (dist < r - 4 && rand() % 15 == 0) { setColor(11); printf("+"); } 
                    else if (dist < r - 4 && rand() % 25 == 0) { setColor(13); printf("."); }
                    else printf(" "); 
                } 
            } 
            printf("\n"); 
        } 
        Sleep(15); // 퍼지는 속도
    } 

    Sleep(1000);
    drawSupermassiveGalaxy(); // 엔딩 그림 호출
} 

void drawSupermassiveGalaxy() {
    system("cls"); 
    printf("\n"); 

    char* galaxyArt[] = {
    "\t                                                   .  * .",
    "\t      * ...::::::...",
    "\t               +                           ..::::;;;;;;;;::::..",
    "\t                                     ..:::;;;@@@@@@@@@@@@;;;:::..", 
    "\t                                  .:::;;@@@@%%%%%%%%%%%%@@@@;;:::.", 
    "\t                  .            .:::;;@@%%%################%%%@@;;:::.",
    "\t                            .:::;;@@%%%###OOOOOOOOOOOOOOOO###%%%@@;;:::.",
    "\t               * .:::;;@@%%%###OOOOOOWWWWWWWWOOOOO###%%%@@;;:::.",
    "\t                      .:::;;@@%%%###OOOOOWWWWWWWWWWWWWWOOOO###%%%@@;;:::.",
    "\t                   .:::;;@@%%%###OOOOOOWWWWWWWWOOOOO###%%%@@;;:::.",
    "\t                .:::;;@@%%%###OOOOOOOOOOOOOOOO###%%%@@;;:::.",
    "\t             .:::;;@@%%%################%%%@@;;:::.            *",
    "\t          .:::;;@@@@%%%%%%%%%%%%@@@@;;:::.             +",
    "\t        ..:::;;;@@@@@@@@@@@@;;;:::..       .",
    "\t        ..::::;;;;;;;;::::..",
    "\t        ...::::::...                  *",
    "\t        .  * ."
    };

    // 상단 우주 공간
    for (int i = 0; i < 3; i++) { 
        printf("\t"); 
        for (int j = 0; j < 70; j++) {
            int r = rand() % 50; 
            if (r == 0) { setColor(8); printf("."); }
            else if (r == 1) { setColor(11); printf("+"); }
            else if (r == 2) { setColor(15); printf("*"); }
            else { printf(" "); } 
        } 
        printf("\n"); 
    } 

    // 은하 및 배경 융합
    for (int i = 0; i < 17; i++) { 
        for (int j = 0; galaxyArt[i][j] != '\0'; j++) { 
            char c = galaxyArt[i][j]; 

            if (c == ' ' && j > 0) { 
                int r = rand() % 50; 
                if (r == 0) { setColor(8); printf("."); } 
                else if (r == 1) { setColor(11); printf("+"); } 
                else if (r == 2) { setColor(15); printf("*"); } 
                else { setColor(7); printf(" "); } 
            } 
            else { 
                if (c == '.') setColor(1); 
                else if (c == ':') setColor(9); 
                else if (c == ';') setColor(11); 
                else if (c == '@') setColor(11); 
                else if (c == '%') setColor(13); 
                else if (c == '#') setColor(13); 
                else if (c == 'O') setColor(15); 
                else if (c == 'W') setColor(15); 
                else if (c == '*') setColor(15); 
                else if (c == '+') setColor(11); 
                else setColor(7); 
                printf("%c", c);
            } 
        } 
        printf("\n");
    } 

    // 하단 우주 공간
    for (int i = 0; i < 3; i++) {
        printf("\t"); 
        for (int j = 0; j < 70; j++) {
            int r = rand() % 50; 
            if (r == 0) { setColor(8); printf("."); } 
            else if (r == 1) { setColor(11); printf("+"); } 
            else if (r == 2) { setColor(15); printf("*"); } 
            else { printf(" "); } 
        } 
        printf("\n"); 
    } 

    setColor(15); 
    printf("\n\t [ 탄생: 초거대 나선 은하 (The Great Spiral Galaxy) ]\n");
    setColor(7); 
    printf("\n\t  당신은 블랙홀의 위협과 거대한 운석 지대를 뚫고\n");
    printf("\t  무한한 질량의 임계점을 돌파하였습니다.\n"); 
    printf("\t  이제 당신의 파편들이 모여 찬란하게 빛나는 푸른 우주가 되었습니다.\n");

    setColor(11); 
    printf("\n\t  - GAME CLEAR: 다중 우주의 창조주가 되다 -\n"); 
    setColor(7);
} 

void showGameOver() { 
    system("cls"); setColor(12); 
    printf("\n\n\t[ GAME OVER ]\n\n\t당신의 우주는 파멸했습니다...\n");
    setColor(7); 
    printf("\n\t최종 도달 질량: %d\n", playerMass); 
} 

/* ============================================================
 * [메인 프로그램: 루프 관리]
 * ============================================================ */
int main() { // 뼈대 함수
    srand((unsigned int)time(NULL));
    hideCursor(); // 콘솔창 커서를 숨김

    // 메인 게임 무한 루프 (메인 메뉴 ↔ 인게임 ↔ 재시작)
    while (1) {
        // 1. 메인 메뉴 출력 (사용자가 3번을 누르면 break)
        if (showMainMenu() == 0)
            break;

        // 2. 게임 초기 세팅
        selectDifficulty(); // 난이도를 선택받아 설정값을 세팅
        initMap(STAGE_1); // 시작할 때 1단계의 맵과 목표치를 세팅
        isGameOver = 0; // 
        playerMass = 0; // 처음 질량 0

        // 3. 인게임 루프
        while (!isGameOver) { 
            drawMap(); // 
            if (currentStage != MINIGAME_1 && currentStage != STAGE_ENDING) drawHUD(); // 화면 아래 상태바(질량)

            processInput(); // 사용자가 방향키를 눌렀는지 확인하고 내 위치를 갱신
            spawnContinuousItems(); // 시간이 지나면 별이 생기게 맵 배열을 바꿈
            updateBlackHoleChase(); // 블랙홀이 플레이어쪽으로 오게 함

            if (currentStage == STAGE_2) { // 2단계 : 별이 끌려옴
                for (int i = 0; i < ROWS; i++) for (int j = 0; j < COLS; j++) { 
                    if (map[i][j] == '*' && abs(i - playerRow) + abs(j - playerCol) < 5) { // *이 5칸 이내로 가깝다면
                        map[i][j] = ' '; playerMass++; // 별이 끌려와서 점수가 1 오름
                    } 
                } 
            } 

            if (currentStage == STAGE_1 && playerMass >= targetMass) initMap(STAGE_2); // 1단계 -> 2단계
            else if (currentStage == STAGE_2 && playerMass >= targetMass) initMap(STAGE_3); // 2단계 -> 3단계
            else if (currentStage == STAGE_3 && playerMass >= targetMass) runMiniGame1(); // 3단계 -> 운석 피하기 미니게임
            else if (currentStage == STAGE_4 && playerMass >= targetMass) { // 
                currentStage = STAGE_ENDING; // 상태를 엔딩 모드
                isGameOver = 1; // 게임오버 플래그를 1로 강제로 바꿔 엔딩
            } 

            turnCount++; // 무한 루프 한 바퀴를 돌았으니 턴 수(시간)를 1 늘림
            Sleep(50);
        } 

        // 4. 게임 종료 후 결과 화면 출력
        // 클리어
        if (currentStage == STAGE_ENDING)
            playEndingAnimation(); 
        // 실패
        else 
            showGameOver();

        // 5. 재시작 여부
        if (askRestart() == 0)
            break;
    } 

    system("cls");
    setColor(7);
    printf("\n\n\t게임을 플레이해주셔서 감사합니다!\n\n");
    return 0; 
} 