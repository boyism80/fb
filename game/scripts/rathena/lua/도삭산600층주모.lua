{
    set @mysd, mysd();
    image 0, 15, 3;
    set @list, list1("안녕하세요. 어떻게 오셨나요?", "물건 사기", "물건 팔기", "금전 맡기기", "금전 되찾기", "물건 맡기기", "물건 되찾기");
    if(@list == 1) callshop 0, 34;
    else if(@list == 2) callshop 1, 35;
    else if(@list == 3) calldeposit 0;
    else if(@list == 4) callwithdraw 0;
    else if(@list == 5) calldeposit 1;
    else if(@list == 6) callwithdraw 1;
}