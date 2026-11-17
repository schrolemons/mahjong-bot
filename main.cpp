#pragma GCC optimize("Ofast")
#include <bits/stdc++.h>
#include "MahjongGB/MahjongGB.h"
using namespace std;
#define SIMPLEIO 1
int id, quan;

class card
{
public:
    int kind, num; //kind为种类，num为点数
    //0万，1条，2筒，3风，4三元
    card(string s)
            : num(s[1] - '0')
    {
        switch (s[0])
        {
            case 'W':
                kind = 0;
                break;
            case 'T':
                kind = 1;
                break;
            case 'B':
                kind = 2;
                break;
            case 'F':
                kind = 3;
                break;
            case 'J':
                kind = 4;
                break;
        }
    }
    card()
            :kind(0),num(0)
    {

    }
    card(int a,int b)
            :kind(a),num(b)
    {

    }
    bool operator<(const card &that) const
    {
        if (kind == that.kind)
            return num < that.num;
        return kind < that.kind;
    }
    string IntToString() const
    {
        string s;
        switch(kind)
        {
            case 0:
                s.push_back('W');
                break;
            case 1:
                s.push_back('T');
                break;
            case 2:
                s.push_back('B');
                break;
            case 3:
                s.push_back('F');
                break;
            case 4:
                s.push_back('J');
                break;
        }
        s.push_back(num + '0');
        return s;
    }
};

int hand[5][10], down[5][10]; //手牌和门前的牌
const int maxn[5]={9,9,9,4,3}; //每种牌的数量
int leave[5][10]; //每个种类未出现的牌数
int maxvalue;     // 最终最优牌型的估价
int AimCard[5][10]; //局部的最优目标牌型
int GO[5][10]; //目标牌型
int lastbest[5][10];
int PaiQiang[4]={21,21,21,21}; //牌墙剩余的牌数
bool gangflag;
bool HaiDi; //是否海底
card pre; //记录上回合打出的牌
card pre2;
vector<pair<string, pair<string, int>>> pack;
int threshold = 0.95 * (double)CLOCKS_PER_SEC;
int start_time; //卡时
bool chiflag; //用于判断是否吃成功

void init();
void play(int);
void MoPai();
void chi(int);
void peng(int);
void gang(int);
void bugang(int);
bool JudgeHu(card,bool,bool);
int getss();
void FindBest();
void ChuPai();
void dfs(int,int,int);
bool cmp(const card &a,const card &b)
{
    return leave[a.kind][a.num]>leave[b.kind][b.num];
}
int main()
{
    init();
    int num,nowid;
    string op;
    while(true)
    {
        threshold = 0.95 * (double)CLOCKS_PER_SEC;
        cin>>num;
        start_time = clock();
        if(num==2)
            MoPai();
        else
        {
            cin>>nowid>>op;
            if(chiflag)
            {
                if(op=="PENG"||op=="GANG")
                {
                    card midcard(pack.back().second.first);
                    hand[midcard.kind][midcard.num-1]++;
                    hand[midcard.kind][midcard.num]++;
                    hand[midcard.kind][midcard.num+1]++;
                    hand[pre.kind][pre.num]--;
                    hand[pre2.kind][pre2.num]++;
                    hand[midcard.kind][0]+=3;
                }
                chiflag=false;
            }
            if(op=="DRAW")
            {
                PaiQiang[nowid]--;
                cout<<"PASS"<<endl;
            }
            else if(op=="PLAY")
                play(nowid);
            else if(op=="PENG")
                peng(nowid);
            else if(op=="CHI")
                chi(nowid);
            else if(op=="GANG")
                gang(nowid);
            else
                bugang(nowid);
        }
        cout<<">>>BOTZONE_REQUEST_KEEP_RUNNING<<<"<<endl;
    }
}

void init()
{
    MahjongInit();
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    string tmp;
    for (int i = 0; i < 5; i++)
        for (int j = 1; j <=maxn[i]; j++)
            leave[i][j] = 4;
    int num;
    cin >> num;
    cin >> num >> id >> quan;
    cout << "PASS" << endl;
    cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
    cin >> num;
    for (int i = 0; i < 4; i++)
        cin >> num;
    for (int i = 0; i < 13; i++)
    {
        cin >> tmp;
        card tmp2(tmp);
        hand[tmp2.kind][tmp2.num]++;
        leave[tmp2.kind][tmp2.num]--;
        hand[tmp2.kind][0]++;
    }
    cout << "PASS" << endl;
    cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
}

void peng(int nowid) //其他玩家碰牌
{
    if(id!=nowid)
        leave[pre.kind][pre.num]-=2;
    play(nowid);
}
void chi(int nowid) //其他玩家吃牌
{
    string midcard;
    string now;
    cin>>midcard;
    if(id!=nowid)
    {
        card tmp1(midcard);
        leave[tmp1.kind][tmp1.num+1]--;
        leave[tmp1.kind][tmp1.num]--;
        leave[tmp1.kind][tmp1.num-1]--;
        leave[pre.kind][pre.num]++;
    }
    play(nowid);
}
void gang(int nowid) //其他玩家杠牌
{
    cout<<"PASS"<<endl;
    gangflag=true;
}
void bugang(int nowid) //其他玩家补杠
{
    string now;
    cin>>now;
    if(id!=nowid)
    {
        card tmp(now);
        if(JudgeHu(tmp,false,true))
        {
            cout<<"HU"<<endl;
            return;
        }
        leave[tmp.kind][tmp.num]--;
        gangflag=true;
    }
    cout<<"PASS"<<endl;
}

int getss() //计算估价
{
    vector<string> nowhand;
    for(int i=0;i<5;i++)
        for(int j=1;j<=maxn[i];j++)
        {
            card now(i,j);
            nowhand.insert(nowhand.end(),AimCard[i][j],now.IntToString());
        }
    string winTile=nowhand.back();
    nowhand.pop_back();
    vector<pair<int,string>>tmpp;
    try {
        tmpp=MahjongFanCalculator(pack,nowhand,winTile,0,false,false,false,false,id,quan);
    }
    catch (...)
    {
        return -99999999;
    }
    int sum=0;
    for(const auto& i:tmpp)
        sum+=i.first;
    if(sum<8)
        return -99999999;
    int ans=10000;
    for(int i=0;i<5;i++)
    {
        for(int j=1;j<=maxn[i];j++)
        {
            if(hand[i][j]>=AimCard[i][j])
                ans+=100;
            else
            {
                int cha=AimCard[i][j]-hand[i][j];
                if(leave[i][j]<cha)
                    return -99999999;
                ans-=(1000/leave[i][j])<<cha;
            }
        }
    }
    return ans*(sum+7200);
}
// 分配面子

void dfs(int HuaSe,int num,int need)
{
    if(clock()-start_time>threshold)
        return;
    if(!need)
    {
        //计算
        int value=getss();
        if(value>maxvalue)
        {
            maxvalue=value;
            for(int i=0;i<5;i++)
                for(int j=0;j<=maxn[i];j++)
                    GO[i][j]=AimCard[i][j];
        }
        return;
    }
    // 枚举刻字
    for (int i = HuaSe; i < 5; i++)
    {
        int j=1;
        if(i==HuaSe)
            j=num;
        for(;j<=maxn[HuaSe];j++)
        {
            if(AimCard[i][j]+3<5&&AimCard[i][j]-hand[i][j]+3<2)
            {
                AimCard[i][0] += 3;
                AimCard[i][j] += 3;
                dfs(i, j, need-1);
                AimCard[i][0] -= 3;
                AimCard[i][j] -= 3;
                if(clock()-start_time>threshold)
                    return;
            }
        }
    }
    if(HuaSe>2)
        return;
    // 枚举顺子
    for (int i =HuaSe; i <3; i++)
    {
        int j=1;
        if(i==HuaSe)
            j=num;
        for(;j<=7;j++)
        {
            if(AimCard[i][j]<5&&AimCard[i][j+1]<5&&AimCard[i][j+2]<5)
            {
                if(AimCard[i][j]<hand[i][j]||AimCard[i][j+1]<hand[i][j+1]||AimCard[i][j+2]<hand[i][j+2])
                {
                    AimCard[i][0] += 3;
                    AimCard[i][j]++;
                    AimCard[i][j+1]++;
                    AimCard[i][j+2]++;
                    dfs(i, j,need-1);
                    AimCard[i][0] -= 3;
                    AimCard[i][j]--;
                    AimCard[i][j + 1]--;
                    AimCard[i][j + 2]--;
                    if(clock()-start_time>threshold)
                        return;
                }
            }
        }
    }
}

void ChuPai()
{
    int discard_only[5][10];
    double canshu[5][10];
    FindBest();
    for (int i=0;i<5;i++)
    {
        for(int j=1;j<=maxn[i];j++)
        {
            if(GO[i][j]>lastbest[i][j])
                canshu[i][j]=2;
            else if(GO[i][j]==lastbest[i][j])
                canshu[i][j]=1.5;
            else
                canshu[i][j]=1;
            if(GO[i][j]==0&&hand[i][j])
                canshu[i][j]=1;
            discard_only[i][j]=0;
        }
    }
    int ii=-1,jj=-1,min=999999;

    for (int i=0;i<5;i++){
        for(int j=1;j<=maxn[i];j++)
        {
            if(ii==-1&&hand[i][j]){
                ii=i;
                jj=j;
            }
            if(hand[i][j]<=GO[i][j]||hand[i][j]==0)
                continue;
            if (discard_only[i][j] == 0&&hand[i][j])
            {
                double start =round((2*leave[i][j]  - 3 *(hand[i][j]-GO[i][j])) * canshu[i][j]);  // 初始值，越低越容易被打出 保留参数口
                discard_only[i][j] += (int)start;
            }
            if (j == 1)
                discard_only[i][j] += hand[i][j + 1] + hand[i][j + 2];
            else if (j == 9)
                discard_only[i][j] += hand[i][j - 1] + hand[i][j - 2];
            else if (j== 8)
                discard_only[i][j] += 2 * hand[i][j + 1] + 2 * hand[i][j - 1] + hand[i][j - 2];
            else if (j == 2)
                discard_only[i][j] += 2 * hand[i][j - 1] + 2 * hand[i][j + 1] + hand[i][j + 2];
            else
                discard_only[i][j] += 3 * hand[i][j - 1] + 3 * hand[i][j + 1] + hand[i][j + 2] + hand[i][j - 2];
            if(discard_only[i][j]<min)
            {
                min=discard_only[i][j];
                ii=i;
                jj=j;
            }
        }
    }
    card tmp;
    tmp.kind = ii;
    tmp.num = jj;
    string playcard = tmp.IntToString();
    hand[ii][jj]--;
    hand[ii][0]--;
    cout << playcard << endl;
    pre2=tmp;
}
void FindBest()
{
    if(pack.empty()) //判断是否可能有特殊牌型
    {
        int DuiZi=0,YaoJiu=0;
        for(int i=0;i<5;i++)
        {
            for(int j=1;j<=maxn[i];j++)
            {
                DuiZi+=hand[i][j]/2;
                if(hand[i][j]&&(i>2||j==1||j==9))
                    YaoJiu++;
            }
        }
        if(DuiZi>=5) //七对
        {
            int nowuse=0;
            vector<card> wating;
            for(int i=0;i<5;i++)
            {
                for(int j=1;j<=maxn[i];j++)
                {
                    if(hand[i][j]==4)
                    {
                        GO[i][j]=AimCard[i][j]=4;
                        nowuse+=4;
                    }
                    else if(hand[i][j]>=2)
                    {
                        GO[i][j]=AimCard[i][j]=2;
                        nowuse+=2;
                    }
                    else if(hand[i][j]==1)
                    {
                        wating.emplace_back(i,j);
                        GO[i][j]=AimCard[i][j]=0;
                    }
                    else
                        GO[i][j]=AimCard[i][j]=0;
                }
            }
            sort(wating.begin(),wating.end(), cmp);
            for(int i=0;i<14-nowuse;i+=2)
                GO[wating[i].kind][wating[i].num]=AimCard[wating[i].kind][wating[i].num]=2;
            maxvalue=getss();
        }
        else if(YaoJiu>=10) //十三幺
        {
            bool tmpflag=false;
            for(int i=0;i<5;i++)
            {
                for(int j=1;j<=maxn[i];j++)
                {
                    if(i>2||j==1||j==9)
                    {
                        GO[i][j]=AimCard[i][j]=1;
                        if(!tmpflag&&hand[i][j]>1)
                        {
                            GO[i][j]=AimCard[i][j]=2;
                            tmpflag=true;
                        }
                    }
                    else
                        GO[i][j]=AimCard[i][j]=0;
                }
            }
            maxvalue=getss();
        }
    }
    for (int i = 0; i < 5; i++)
        for (int j = 1; j <= maxn[i]; j++)
            lastbest[i][j]=GO[i][j];
    // 枚举将牌
    int need =(hand[0][0]+hand[1][0]+hand[2][0]+hand[3][0]+hand[4][0]) / 3;  // 假定面子都为3张
    for (int i = 0; i < 5; i++)
    {
        for (int j = 1; j <= maxn[i]; j++)
        {
            if(clock()-start_time>threshold)
                return;
            if (hand[i][j]>=2)
            {
                AimCard[i][j] = 2;
                AimCard[i][0] = 2;
                dfs(0,1,need);               // 分配面子
                AimCard[i][j] = 0;
                AimCard[i][0] = 0;
            }
        }
    }
    for (int i = 0; i < 5; i++)
    {
        for (int j = 1; j <= maxn[i]; j++)
        {
            if(clock()-start_time>threshold)
                return;
            if (hand[i][j]==1)
            {
                AimCard[i][j] = 2;
                AimCard[i][0] = 2;
                dfs(0,1,need);               // 分配面子
                AimCard[i][j] = 0;
                AimCard[i][0] = 0;
            }
        }
    }
}
void play(int nowid) //其他玩家出牌
{
    string now;
    cin>>now;
    if(nowid==id)
    {
        cout<<"PASS"<<endl;
        return;
    }
    card nowcard(now);
    pre=nowcard;
    leave[nowcard.kind][nowcard.num]--;
    HaiDi=!PaiQiang[(nowid+1)%4];
    if(JudgeHu(nowcard,false,gangflag))
    {
        cout<<"HU"<<endl;
        return;
    }
    gangflag=false;
    //碰牌
    if(HaiDi)
    {
        cout<<"PASS"<<endl;
        return;
    }
    if( hand[nowcard.kind][nowcard.num]>=2&&GO[nowcard.kind][nowcard.num]>hand[nowcard.kind][nowcard.num])
    {
        hand[nowcard.kind][nowcard.num]-=2;
        hand[nowcard.kind][0]-=2;
        int pengid;
        if((nowid+1)%4==id)
            pengid=1;
        else if((nowid+3)%4==id)
            pengid=3;
        else
            pengid=2;
        pack.push_back({"PENG",{now,pengid}});
        maxvalue=-99999999;
        FindBest();
        cout << "PENG ";
        down[nowcard.kind][nowcard.num]+=3;
        down[nowcard.kind][0]+=3;
        ChuPai();
        return;
    }
    else if( hand[nowcard.kind][nowcard.num]==3)
    {
        threshold = 0.45 * (double)CLOCKS_PER_SEC;
        hand[nowcard.kind][nowcard.num]-=3;
        hand[nowcard.kind][0]-=3;
        int gangid;
        if((nowid+1)%4==id)
            gangid=1;
        else if((nowid+3)%4==id)
            gangid=3;
        else
            gangid=2;
        pack.push_back({"GANG",{now,gangid}});
        int oldvalue=maxvalue;
        FindBest();
        if(maxvalue>oldvalue)
        {
            maxvalue=-99999999;
            cout << "GANG" << endl;
            down[nowcard.kind][nowcard.num]+=4;
            down[nowcard.kind][0]+=4;
            return;
        }
        else
        {
            hand[nowcard.kind][nowcard.num]+=3;
            hand[nowcard.kind][0]+=3;
            pack.pop_back();
            for (int i = 0; i < 5; i++)
                for (int j = 1; j <= maxn[i]; j++)
                    GO[i][j] =lastbest[i][j];
            maxvalue=oldvalue;
        }
    }
    else if(nowcard.kind<=2&&(nowid+1)%4==id)
    {
        threshold = 0.9 * (double)CLOCKS_PER_SEC;
        if((nowcard.num>=2&&nowcard.num<=8&&hand[nowcard.kind][nowcard.num-1]>=1&&hand[nowcard.kind][nowcard.num+1]>=1)||(nowcard.num<=7&&hand[nowcard.kind][nowcard.num+1]>=1&&hand[nowcard.kind][nowcard.num+2]>=1)||(nowcard.num>=3&&hand[nowcard.kind][nowcard.num-1]>=1&&hand[nowcard.kind][nowcard.num-2]>=1))
        {
            hand[nowcard.kind][nowcard.num]++;
            hand[nowcard.kind][0]++;
            int oldvalue=maxvalue;
            FindBest();
            if(maxvalue>oldvalue){
                cout << "CHI ";
                card tmp;
                tmp.kind=nowcard.kind;
                string midcard;
                if(nowcard.num<=7&& hand[nowcard.kind][nowcard.num+1]>=1&&hand[nowcard.kind][nowcard.num+2]>=1)
                {
                    hand[nowcard.kind][nowcard.num]--;
                    hand[nowcard.kind][nowcard.num+1]--;
                    hand[nowcard.kind][nowcard.num+2]--;
                    tmp.num=nowcard.num+1;
                    midcard=tmp.IntToString();
                }
                else if(nowcard.num>=3&& hand[nowcard.kind][nowcard.num-1]>=1&&hand[nowcard.kind][nowcard.num-2]>=1)
                {
                    hand[nowcard.kind][nowcard.num]--;
                    hand[nowcard.kind][nowcard.num-1]--;
                    hand[nowcard.kind][nowcard.num-2]--;
                    tmp.num=nowcard.num-1;
                    midcard=tmp.IntToString();
                }
                else if(2<=nowcard.num&&nowcard.num<=8&& hand[nowcard.kind][nowcard.num+1]>=1&&hand[nowcard.kind][nowcard.num-1]>=1)
                {
                    hand[nowcard.kind][nowcard.num]--;
                    hand[nowcard.kind][nowcard.num+1]--;
                    hand[nowcard.kind][nowcard.num-1]--;
                    tmp.num=nowcard.num;
                    midcard=tmp.IntToString();
                }
                cout<<midcard<<" ";
                int tmp2;
                if(tmp.num==nowcard.num+1)
                    tmp2=1;
                else if(tmp.num==nowcard.num)
                    tmp2=2;
                else
                    tmp2=3;
                pack.push_back({"CHI",{midcard,tmp2}});
                maxvalue=-99999999;
                hand[nowcard.kind][0]-=3;
                chiflag=true;
                ChuPai();
                return;
            }
            else
            {
                hand[nowcard.kind][nowcard.num]--;
                hand[nowcard.kind][0]--;
                for (int i = 0; i < 5; i++)
                    for (int j = 1; j <= maxn[i]; j++)
                        GO[i][j] =lastbest[i][j];
                maxvalue=oldvalue;
            }
        }
    }
    cout<<"PASS"<<endl;
}

void MoPai()
{
    string tmp;
    cin>>tmp;
    card tmp2(tmp);
    HaiDi=!PaiQiang[id];
    if (JudgeHu(tmp2,true,false))
    {
        cout << "HU" << endl;
        return;
    }
    hand[tmp2.kind][tmp2.num]++;
    hand[tmp2.kind][0]++;
    leave[tmp2.kind][tmp2.num]--;
    leave[tmp2.kind][0]--;
    FindBest();
    if(!HaiDi)
    {
        if (hand[tmp2.kind][tmp2.num] == 4)
        {
            cout << "GANG" << " " <<tmp<< endl;
            pack.push_back({"GANG",{tmp,0}});
            hand[tmp2.kind][tmp2.num]-=4;
            hand[tmp2.kind][0]-=4;
            down[tmp2.kind][tmp2.num]+=4;
            hand[tmp2.kind][0]+=4;
        }
        else if(down[tmp2.kind][tmp2.num] == 3)
        {
            for(auto & i : pack)
            {
                if(i.second.first==tmp)
                {
                    i.first="GANG";
                    cout<<"BUGANG"<<' '<<tmp<<endl;
                    hand[tmp2.kind][tmp2.num]--;
                    down[tmp2.kind][tmp2.num]++;
                    break;
                }
            }
        }
    }
    cout<<"PLAY"<<" ";
    ChuPai();
}

bool JudgeHu(card now,bool isZIMO,bool isGANG) //判断在获得now以后是否胡牌,isZIMO表示是否为自摸
{
    vector<string> nowhand;
    for(int i=0;i<5;i++)
        for(int j=1;j<=9;j++)
        {
            card now(i,j);
            nowhand.insert(nowhand.end(),hand[i][j],now.IntToString());
        }
    string winTile=now.IntToString();
    int flowerCount=0;
    bool isJUEZHANG=!leave[now.kind][now.num];
    vector<pair<int,string>>tmpp;
    try {
        tmpp=MahjongFanCalculator(pack,nowhand,winTile,flowerCount,isZIMO,isJUEZHANG,isGANG,HaiDi,id,quan);
    }
    catch (...)
    {
        return false;
    }
    int sum=0;
    for(const auto& i:tmpp)
    {
        sum+=i.first;
        if(sum>=8)
            return true;
    }
    return false;
}