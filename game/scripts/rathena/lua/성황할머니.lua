{
    set @sd, mysd();
    set @id, myid();

    image 0, 10, 3;

    if(get_state(@sd) == 1)
    {
        set @list1, list1("생명의 소중함을 그렇게\\n일러왔거늘... 앞으로도\\n사소한 일에 목숨을 걸지\\n않으리라고 내가 어떻게 믿을\\n수 있겠느냐? 또 생명을 잃고\\n나를 찾아오지 않겠다고\\n맹세할 수 있겠느냐?", "예", "아니오");
        if(@list1 == 1)
        {
            set @list2, list1("싸움은 싸움을 부르고, 피는\\n반드시 피를 보게 되느니라,\\n이번의 죽음도 네 책임이라는\\n것을 진심으로 느끼고 반성하고 있느냐?", "예", "아니오");
            if(@list2 == 1)
            {
                set @list3, list1( "그렇다면 잃은 물건과\\n경험치도 다 네 욕심에서\\n비롯되었음을 인정하겠느냐?", "예", "아니오");
                if(@list3 == 1)
                {
                    set @list4, list1( "네가 새로 생명을 얻게\\n되더라도 절대로 무고한\\n생명을 해치지 않을 것을\\n맹세하느냐?", "예", "아니오");
                    if(@list4 == 1) 
                    { 
                        set_state @sd, 0;
                        if(get_equipitem(@sd, 6) == name2spellid("보무의목걸이") || get_equipitem(@sd, 7) == name2itemid("보무의목걸이"))
                        {
                            spellcast @sd, (name2spellid("자동보호"));
                            spellcast @sd, (name2spellid("자동무장"));
                        }
                        if(get_hp(@sd) <= 0) { set_hp @sd, 1; }
                        updatelookpc @sd, 1;
                        set @list5, list1( "너의 각오를 믿고 새로운\\n생명을 내리노니 나에게 한\\n맹세를 잊지 말고 하루\\n하루를 신께 감사하는\\n마음으로 살아가도록 하여라.");
                    }
                }
            }
        }
    }
}