#include<bits/stdc++.h>
using namespace std;
#define int long long
int power(int x,int n){
    int ans=1;
    for(int i=0;i<n;i++){
        ans*=x;
    }
    return ans;
}

vector<int> pc;
vector<int> inst;
vector<int> op;
int cnt=0;
map<int,pair<int,pair<string,string>>> buff;
map<int,string> sttbl;

int hex2dec(string hexVal){ 
    int len=hexVal.size(); 
    int base=1;   
    int dec_val=0;    
    for(int i=len-1; i>=0; i--){ 
        if (hexVal[i]>='0'&&hexVal[i]<='9'){ 
            dec_val+=((int)(hexVal[i])-48)*base;
            base=base*16; 
        } 
        else if(hexVal[i]>='a'&&hexVal[i]<='f'){ 
            dec_val+=(int)((hexVal[i])-(int)('a')+10)*base; 
            base=base*16; 
        } 
    } 
    return dec_val; 
}

void read_pc(string s){
    int n=0;
    ifstream file(s);
    if(!file){
        cout<<"Error in opening file!\n";
        exit(0);
        return;
    }
    int p, x;
    string line;
    while(getline(file,line)){
        n++;
        if(line.length()==0){
            continue;
        }
        string s1, s2;
        int i=0;
        while(i<line.length()&&line[i]!='x'){
            i++;
        }
        if(i>=line.length()-1){
            continue;
        }
        i++;
        for(int j=0;j<8;j++){
            s1+=line[i];
            i++;
            if(i>=line.length()-1){
                break;
            }
        }
        if(i>=line.length()-1){
            continue;
        }
        while(i<line.length()&&line[i]!='x'){
            i++;
        }
        if(i>=line.length()-1){
            continue;
        }
        i++;
        int f=0;
        for(int j=0;j<8;j++){
            if(i==line.length()){
                f=1;
                break;
            }
            s2+=line[i];
            i++;
            if(i>line.length()){
                break;
            }
        }
        if(i>line.length()||f==1){
            continue;
        }
        p=hex2dec(s1);
        x=hex2dec(s2);
        pc.push_back(p);
        inst.push_back(x);
    }
}

void conv(){
    for(int i=0;i<inst.size();i++){
        op.push_back(inst[i]%128);
    }
}

bool isbranch(int x){
    if(x==99||x==103||x==111){
        return true;
    }
    return false;
}

void count(){
    for(int i=0;i<op.size();i++){
        if(op[i]==99||op[i]==103||op[i]==111){
            cnt++;
        }
    }
}

int calc(bitset<32> a,int id){
    int ans=0;
    if(op[id]==99){
        if(a[12]==1){
            ans-=power(2,12);
        }
        for(int i=0;i<12;i++){
            if(a[i]==1){
                ans+=power(2,i);
            }
        }
    }
    else if(op[id]==111){
        if(a[20]==1){
            ans-=power(2,20);
        }
        for(int i=0;i<20;i++){
            if(a[i]==1){
                ans+=power(2,i);
            }
        }
    }
    return ans;
}


int extract(int id){
    bitset<32> a=inst[id];
    bitset<32> temp;
    if(op[id]==99){
        temp[11]=a[7];
        for(int i=1;i<=4;i++){
            temp[i]=a[7+i];
        }
        for(int i=5;i<=10;i++){
            temp[i]=a[20+i];
        }
        temp[12]=a[31];
    }
    else if(op[id]==111){
        for(int i=12;i<=19;i++){
            temp[i]=a[i];
        }
        temp[11]=a[20];
        for(int i=1;i<=10;i++){
            temp[i]=a[20+i];
        }
        temp[20]=a[31];
    }
    else if(op[id]==103){
        return pc[id+1]-pc[id];
    }
    int ans=calc(temp,id);
    return ans;
}

float alwaystaken(){
    int corr=0;
    for(int i=0;i<op.size()-1;i++){
        if(isbranch(op[i])){
            if(buff.find(pc[i])==buff.end()){
                buff[pc[i]].first=pc[i]+extract(i);
            }
            if(pc[i+1]!=pc[i]+4){
                corr++;
                buff[pc[i]].second.first.push_back('T');
                buff[pc[i]].second.second.push_back('T');
            }
            else{
                buff[pc[i]].second.first.push_back('N');
                buff[pc[i]].second.second.push_back('T');
            }
        }
    }
    return (corr*100.0)/cnt;
}

float alwaysnottaken(){
    int corr=0;
    for(int i=0;i<op.size()-1;i++){
        if(isbranch(op[i])){
            if(buff.find(pc[i])==buff.end()){
                buff[pc[i]].first=pc[i]+extract(i);
            }
            if(pc[i+1]!=pc[i]+4){
                buff[pc[i]].second.first.push_back('T');
                buff[pc[i]].second.second.push_back('N');
            }
            else{
                corr++;
                buff[pc[i]].second.first.push_back('N');
                buff[pc[i]].second.second.push_back('N');
            }
        }
    }
    return (corr*100.0)/cnt;
}

float oneb(){
    int corr=0;
    for(int i=0;i<op.size()-1;i++){
        if(isbranch(op[i])){
            if(buff.find(pc[i])==buff.end()){
                buff[pc[i]].first=pc[i]+extract(i);
                sttbl[pc[i]]="N";
            }
            if(pc[i+1]!=pc[i]+4){
                buff[pc[i]].second.first.push_back('T');
                if(sttbl[pc[i]]=="N"){
                    buff[pc[i]].second.second.push_back('N');
                    sttbl[pc[i]]="T";
                }
                else if(sttbl[pc[i]]=="T"){
                    corr++;
                    buff[pc[i]].second.second.push_back('T');
                }
            }
            else{
                buff[pc[i]].second.first.push_back('N');
                if(sttbl[pc[i]]=="N"){
                    corr++;
                    buff[pc[i]].second.second.push_back('N');
                }
                else if(sttbl[pc[i]]=="T"){
                    sttbl[pc[i]]="N";
                    buff[pc[i]].second.second.push_back('T');
                }
            }
        }
    }
    return (corr*100.0)/cnt;
}

float twob(){
    int corr=0;
    for(int i=0;i<op.size()-1;i++){
        if(isbranch(op[i])){
            if(buff.find(pc[i])==buff.end()){
                buff[pc[i]].first=pc[i]+extract(i);
                sttbl[pc[i]]="SNT";
            }
            if(pc[i+1]!=pc[i]+4){
                buff[pc[i]].second.first.push_back('T');
                if(sttbl[pc[i]]=="SNT"){
                    buff[pc[i]].second.second.push_back('N');
                    sttbl[pc[i]]="WNT";
                }
                else if(sttbl[pc[i]]=="WNT"){
                    buff[pc[i]].second.second.push_back('N');
                    sttbl[pc[i]]="WT";
                }
                else if(sttbl[pc[i]]=="WT"){
                    corr++;
                    buff[pc[i]].second.second.push_back('T');
                    sttbl[pc[i]]="ST";
                }
                else if(sttbl[pc[i]]=="ST"){
                    corr++;
                    buff[pc[i]].second.second.push_back('T');
                }
            }
            else{
                buff[pc[i]].second.first.push_back('N');
                if(sttbl[pc[i]]=="SNT"){
                    corr++;
                    buff[pc[i]].second.second.push_back('N');
                }
                else if(sttbl[pc[i]]=="WNT"){
                    corr++;
                    buff[pc[i]].second.second.push_back('N');
                    sttbl[pc[i]]="SNT";
                }
                else if(sttbl[pc[i]]=="WT"){
                    buff[pc[i]].second.second.push_back('T');
                    sttbl[pc[i]]="WNT";
                }
                else if(sttbl[pc[i]]=="ST"){
                    buff[pc[i]].second.second.push_back('T');
                    sttbl[pc[i]]="WT";
                }
            }
        }
    }
    return (corr*100.0)/cnt;
}

int32_t main(){
    string s;
    cout<<"ENTER THE FILE NAME YOU WANT TO USE: ";
    cin>>s;
    read_pc(s);
    conv();
    count();
    int width=40;
    int op;
    cout<<"Enter the type of predictor you want to use: ";
    cin>>op;
    if(op==1){
        cout<<"The accuracy is of always taken predictor on "<<s<<" is: "<<alwaystaken()<<"%"<<endl;
        cout<<left<<setw(width)<<"PC"<<setw(width)<<"Target Address"<<setw(width)<<"Actual History"<<setw(width)<<"Prediction History"<<endl;
        for(auto it=buff.begin();it!=buff.end();it++){
            cout<<left<<setw(width)<<it->first<<setw(width)<<it->second.first<<setw(width)<<it->second.second.first<<setw(width)<<it->second.second.second<<endl;
        }
        cout<<endl;
    }
    else if(op==2){
        cout<<"The accuracy of always not taken predictor on "<<s<<" is: "<<alwaysnottaken()<<"%"<<endl;
        cout<<left<<setw(width)<<"PC"<<setw(width)<<"Target Address"<<setw(width)<<"Actual History"<<setw(width)<<"Prediction History"<<endl;
        for(auto it=buff.begin();it!=buff.end();it++){
            cout<<left<<setw(width)<<it->first<<setw(width)<<it->second.first<<setw(width)<<it->second.second.first<<setw(width)<<it->second.second.second<<endl;
        }
        cout<<endl;
    }
    else if(op==3){
        cout<<"The accuracy of 1-bit predictor on "<<s<<" is: "<<oneb()<<"%"<<endl;
        cout<<left<<setw(width)<<"PC"<<setw(width)<<"Target Address"<<setw(width)<<"Actual History"<<setw(width)<<"Prediction History"<<endl;
        for(auto it=buff.begin();it!=buff.end();it++){
            cout<<left<<setw(width)<<it->first<<setw(width)<<it->second.first<<setw(width)<<it->second.second.first<<setw(width)<<it->second.second.second<<endl;
        }
        cout<<endl;
    }
    else if(op==4){
        cout<<"The accuracy of 2-bit predictor on "<<s<<" is: "<<twob()<<"%"<<endl;
        cout<<left<<setw(width)<<"PC"<<setw(width)<<"Target Address"<<setw(width)<<"Actual History"<<setw(width)<<"Prediction History"<<endl;
        for(auto it=buff.begin();it!=buff.end();it++){
            cout<<left<<setw(width)<<it->first<<setw(width)<<it->second.first<<setw(width)<<it->second.second.first<<setw(width)<<it->second.second.second<<endl;
        }
        cout<<endl;
    }
    return 0;
}