#include <unistd.h>
 
#include "SemidriveLink.hpp"

class MyListener:public SemidriveLinkListener {
public:
    void onEvent(int32_t event,int32_t value) {
        printf("onEvent is %d,value is %d \n",event,value);
    }

    int getType(){
        return LINK_IVI;
    }
};

int main() {
    MyListener ll;
    SemidriveLink *link = new SemidriveLink(&ll);
    sleep(10);
    printf("test oupdate \n");
    std::map<int,int> values;
    values[0] = 1;
    //values[1] = 1;
    link->updateEvent(values);
    while(1) {sleep(1);
	if(values[0] == 1) {
		values[0] = 0;
	} else {
		values[0] = 1;
	}
    //values[1] = 1;
	printf("write !!! \n");
    link->updateEvent(values);

    }
}
