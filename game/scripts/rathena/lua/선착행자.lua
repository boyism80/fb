{
    if($soul_doll == 6)
    {
COS003:
        set @pos, dialog(0, 0, 1, "하하, 환상의 섬은 정말 즐거운 곳이었어. 그렇지 않나? 난 지금 그곳에서만 구할 수 있다는 특산품을 사러 가는 중일세.");
        if(@pos == 0) end;
COS004:
        set @pos, dialog(0, 1, 1, "특히 그곳에서만 구할 수 있다는 요리를 이야기를 들으니 이거 먹고 싶어서 참을 수가 없더구만. 허허...");
        if(@pos == 0) end;
        else if(@pos == -1) goto COS003;

        if($soul_doll == 6) { set $soul_doll, 7; }
        end;
    }
COS001:
    set @pos, dialog(0, 0, 1, "여긴 환상의 섬으로 가는 선착장이 아니지. 아직도 길을 모르는 사람이 있더군. 여기서 남서쪽으로 가야 한다네.");
    if(@pos == 0) end;
COS002:
    set @pos, dialog(0, 1, 1, "그럼 샛길이 나오는데, 좀 더 걸어가면 또 다른 선착장이 나올 거야. 거기서 배를 탈 수 있다네!");
    if(@pos == 0) end;
    else if(@pos == -1) goto COS001;

    if($soul_doll == 2) { set $soul_doll, 3; }
}