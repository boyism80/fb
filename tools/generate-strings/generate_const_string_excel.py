#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
model.h에서 모든 MESSAGE_* 상수를 추출하여 엑셀 파일을 생성하는 스크립트
"""

import re
import os
from openpyxl import Workbook
from openpyxl.styles import Font


def extract_messages_from_model_h(file_path):
    """
    model.h 파일에서 모든 MESSAGE_* 상수를 추출합니다.
    
    Returns:
        list: [(상수명, 문자열), ...]
    """
    messages = []
    pattern = re.compile(r'inline static constexpr const char\* (MESSAGE_\w+) = "([^"]+)";')
    
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                const_name = match.group(1)
                message = match.group(2)
                messages.append((const_name, message))
    
    return messages


def categorize_message(const_name):
    """
    상수명을 기반으로 시트명을 결정합니다.
    
    Returns:
        str: 시트명
    """
    # 로그인 서버 관련
    if const_name.startswith('MESSAGE_ACCOUNT_'):
        return 'string.login'
    
    # 게이트웨이 관련
    if const_name.startswith('MESSAGE_CLIENT_'):
        return 'string.gateway'
    
    # 아이템 관련
    if const_name.startswith('MESSAGE_ITEM_'):
        return 'string.game.item'
    
    # 돈 관련
    if const_name.startswith('MESSAGE_MONEY_'):
        return 'string.game.money'
    
    # 장비 관련
    if const_name.startswith('MESSAGE_EQUIPMENT_'):
        return 'string.game.equipment'
    
    # 그룹 관련
    if const_name.startswith('MESSAGE_GROUP_'):
        return 'string.game.group'
    
    # 클랜 관련
    if const_name.startswith('MESSAGE_CLAN_'):
        return 'string.game.clan'
    
    # 경험치, 로그인 경과 시간
    if const_name.startswith('MESSAGE_EXP_') or const_name.startswith('MESSAGE_LOGIN_ELAPSED_') or const_name == 'MESSAGE_LEVEL_UP':
        return 'string.game.character'
    
    # 맵, 워프 관련
    if const_name.startswith('MESSAGE_MAP_') or const_name.startswith('MESSAGE_WARP_'):
        return 'string.game.map'
    
    # 스펠 관련
    if const_name.startswith('MESSAGE_SPELL_'):
        return 'string.game.spell'
    
    # 통신 관련
    if const_name.startswith('MESSAGE_WHISPER_'):
        return 'string.game.communication'
    
    # UI/옵션 관련
    if const_name.startswith('MESSAGE_OPTION_'):
        return 'string.game.ui'
    
    # 보관함 관련
    if const_name.startswith('MESSAGE_STORAGE_'):
        return 'string.game.storage'
    
    # 게시판 관련
    if const_name.startswith('MESSAGE_BULLETIN_') or const_name == 'MESSAGE_WRITE_BULLETIN_FAILED':
        return 'string.game.bulletin'
    
    # 우편 관련
    if const_name.startswith('MESSAGE_MAIL_'):
        return 'string.game.bulletin'
    
    # 에셋 로딩 관련
    if const_name.startswith('MESSAGE_ASSET_'):
        return 'string.game.asset'
    
    # 예외 관련
    if const_name.startswith('MESSAGE_EXCEPTION_'):
        return 'string.game.exception'
    
    # 교환 관련
    if const_name.startswith('MESSAGE_TRADE_'):
        return 'string.game.trade'
    
    # 조합 관련
    if const_name.startswith('MESSAGE_MIX_'):
        return 'string.game.mix'
    
    # 말 타기 관련
    if const_name.startswith('MESSAGE_RIDE_'):
        return 'string.game.ride'
    
    # 문 관련
    if const_name.startswith('MESSAGE_DOOR_'):
        return 'string.game.door'
    
    # 기타 (에러, 유저, 메일 등)
    if const_name.startswith('MESSAGE_ERROR_') or const_name.startswith('MESSAGE_UNKNOWN_') or \
       const_name.startswith('MESSAGE_NOT_') or const_name.startswith('MESSAGE_INVALID_') or \
       const_name.startswith('MESSAGE_NO_') or const_name.startswith('MESSAGE_HAVE_') or \
       const_name.startswith('MESSAGE_ALREADY_') or const_name.startswith('MESSAGE_CANNOT_') or \
       const_name.startswith('MESSAGE_TOO_') or \
       const_name.startswith('MESSAGE_REPAIR_') or const_name.startswith('MESSAGE_WEAPON_') or \
       const_name.startswith('MESSAGE_WEAWPON_') or const_name == 'MESSAGE_NOT_READY_GAME_SERVER' or \
       const_name.startswith('MESSAGE_USER_'):
        return 'string.game.etc'
    
    # 기본값: 기타
    return 'string.game.etc'


def create_excel_file(sheets_data, output_path):
    """
    시트 데이터를 사용하여 엑셀 파일을 생성합니다.
    
    Args:
        sheets_data: {시트명: [(상수명, 서버, 타입, 문자열), ...]}
        output_path: 출력 엑셀 파일 경로
    """
    wb = Workbook()
    
    # 기본 시트 제거
    if 'Sheet' in wb.sheetnames:
        wb.remove(wb['Sheet'])
    
    # 폰트 스타일 정의 (사이즈 10으로 통일)
    default_font = Font(size=10)
    
    # 시트명으로 정렬하여 순서대로 생성
    for sheet_name in sorted(sheets_data.keys()):
        data = sheets_data[sheet_name]
        ws = wb.create_sheet(title=sheet_name)
        
        # 데이터 작성
        for row_idx, (const_name, server, data_type, message) in enumerate(data, start=1):
            ws.cell(row=row_idx, column=1, value=const_name).font = default_font
            ws.cell(row=row_idx, column=2, value=server).font = default_font
            ws.cell(row=row_idx, column=3, value=data_type).font = default_font
            ws.cell(row=row_idx, column=4, value=message).font = default_font
        
        # 열 너비 조정
        ws.column_dimensions['A'].width = 40  # 상수명
        ws.column_dimensions['B'].width = 12  # 서버
        ws.column_dimensions['C'].width = 10  # 타입
        ws.column_dimensions['D'].width = 60  # 문자열
    
    # 출력 디렉토리 생성
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # 파일 저장
    wb.save(output_path)
    print(f"엑셀 파일이 생성되었습니다: {output_path}")
    print(f"총 {len(sheets_data)}개의 시트가 생성되었습니다.")


def main():
    """메인 함수"""
    # 스크립트가 tools/generate-strings/에서 실행되므로 루트 디렉토리로 이동
    script_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.join(script_dir, '..', '..')
    model_h_file = os.path.join(root_dir, 'include', 'fb', 'model', 'model.h')
    output_file = os.path.join(root_dir, 'resources', 'table', 'const.string.xlsx')
    
    try:
        # 메시지 추출
        print(f"model.h에서 메시지 추출 중...")
        messages = extract_messages_from_model_h(model_h_file)
        print(f"추출된 메시지 수: {len(messages)}개")
        
        # 시트별로 분류
        sheets_data = {}
        for const_name, message in messages:
            sheet_name = categorize_message(const_name)
            if sheet_name not in sheets_data:
                sheets_data[sheet_name] = []
            sheets_data[sheet_name].append((const_name, 'server', 'string', message))
        
        # 통계 출력
        total_messages = sum(len(data) for data in sheets_data.values())
        print(f"\n시트별 메시지 수:")
        for sheet_name in sorted(sheets_data.keys()):
            print(f"  - {sheet_name}: {len(sheets_data[sheet_name])}개")
        print(f"\n총 메시지 수: {total_messages}개")
        print(f"총 시트 수: {len(sheets_data)}개")
        
        # 엑셀 파일 생성
        print(f"\n엑셀 파일 생성 중...")
        create_excel_file(sheets_data, output_file)
        
        print("\n완료!")
        
    except FileNotFoundError:
        print(f"오류: {model_h_file} 파일을 찾을 수 없습니다.")
    except Exception as e:
        print(f"오류 발생: {e}")
        import traceback
        traceback.print_exc()


if __name__ == '__main__':
    main()

