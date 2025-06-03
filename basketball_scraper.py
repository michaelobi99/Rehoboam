from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from typing import List
from time import sleep
from contextlib import suppress
from io import StringIO
from concurrent.futures import ThreadPoolExecutor, as_completed


def setup_driver():
    options = webdriver.ChromeOptions()
    options.add_argument('--disable-notifications')
    options.add_argument('--headless')  # Run in background
    options.add_argument('--disable-gpu')  # Recommended for headless
    options.add_argument('--window-size=1920,1080')  # Set a standard window size
    options.add_argument('--no-sandbox')  # Bypass OS security model
    options.add_argument('--disable-dev-shm-usage')  # Overcome limited resource problems
    
    # Add a realistic user agent
    options.add_argument('--user-agent=Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36')
    
    # Some additional useful options
    options.add_argument('--disable-blink-features=AutomationControlled')  # Hide automation
    options.add_experimental_option('excludeSwitches', ['enable-automation'])  # Hide automation 
    options.add_experimental_option('useAutomationExtension', False)  # Hide automation
    
    driver = webdriver.Chrome(options=options)
    
    # Execute JS to modify navigator.webdriver flag
    driver.execute_script("Object.defineProperty(navigator, 'webdriver', {get: () => undefined})")
    
    return driver


def is_game_live(game_element):
    try:
        game_element.find_element(By.CLASS_NAME, "event__stage")
        return True
    except NoSuchElementException:
        return False
    

def get_league_name_and_country(header_text):
    """
    Extracts the league name from header text like 'USA : NCAA Standings' or 'EUROPE : Eurocup Standings'
    Returns just the league name (e.g., 'NCAA' or 'Eurocup')
    """
    try:
        parts = header_text.strip().split(':')
        if len(parts) > 1:
            country = parts[0].strip()
            league = parts[1].strip().split('Standings')[0].strip()
            return (league, country)
        return (header_text.strip(), "")
    except:
        return (header_text.strip(), "")
    

def get_exact(league_name):
    if league_name == "NBL1 East" or league_name == "NBL1 South" or league_name == "NBL1 West" \
        or league_name == "NBL1 Central": return "NBL1"
    if league_name == "NBL1 East Women" or league_name == "NBL1 South Women" or \
        league_name == "NBL1 West Women" or league_name == "NBL1 Central Women" or \
        league_name == "NBL1 North" or league_name == "NBL1 North Women": return "NBL1"
    if league_name == "Premier League": return "PL"
    if league_name == "Premier League - Play Offs": return "PL"
    if league_name == "Premier League Women": return "PRE"
    if league_name == "Premier League Women - Play Offs": return "PRE"
    if league_name == "Prva Liga" or league_name == "Prva Liga - Play Offs": return "PL"
    if league_name == "Pro B" or league_name == "Pro B - Play - In": return "PB"
    if league_name == "Pro B - Play Offs": return "PB"
    # if league_name == "Pro B": return "PROB"
    if league_name == "Pro A" or league_name == "Pro A - Play Offs": return "PA"
    if league_name == "Super League": return "SL"
    if league_name == "Super League - Promotion - Play Offs": return "SL"
    if league_name == "Super League - Play Offs": return "SL"
    if league_name == "Super League - Relegation Group": return "SL"
    if league_name == "Liga Leumit" or league_name == "Liga Leumit - Losers stage" \
        or league_name == "Liga Leumit - Winners stage": return "LL"
    if league_name == "Super Lig": return "SL"
    if league_name == "Super Lig - Play Offs": return "SL"
    if league_name == "Premijer liga" or league_name == "Premijer liga - Play Offs": return "A1"
    if league_name == "First League": return "FL"
    if league_name == "Basket Liga" or league_name == "Basket Liga - Play off" or \
        league_name == "Basket Liga - Play Offs": return "BL"
    if league_name == "Basket League": return "BL"
    if league_name == "SB League" or league_name == "SB League - Play Offs": return "SBL"
    if league_name == "Basketligan" or league_name == "Basketligan - Play Offs": return "LIG"
    if league_name == "Liga A" or league_name == "Liga A - Play Offs" or league_name == "Liga A - Play out": return "LA"
    if league_name == "Liga Uruguaya" or league_name == "Liga Uruguaya - Winners stage" or \
        league_name == "Liga Uruguaya - Losers stage" or league_name == "Liga Uruguaya - Play Offs":
        return "LC"
    if league_name == "Superleague" or league_name == "Superleague - Play Offs": return "SL"
    if league_name == "Superliga" or league_name == "Superliga - Play Offs": return "SL"
    # if league_name == "Superliga": return "ABL"
    # if league_name == "Superliga - Play Offs": return "ABL"
    # if league_name == "Superliga - Losers stage": return "ABL"
    # if league_name == "Superliga - Winners stage": return "ABL"
    if league_name == "NB I. A": return "NBI"
    if league_name == "NB I. A - 5th-8th places": return "NBI"
    if league_name == "NB I. A - Play Offs": return "NBI"
    if league_name == "NB I. A - Play Out": return "NBI"
    if league_name == "NB I. A Women": return "DIV"
    if league_name == "NB I. A Women - Play Offs": return "DIV"
    if league_name == "Lega A" or league_name == "Lega A - Play Offs": return "LA"
    if league_name == "Serie A2": return "A2"
    if league_name == "SLB" or league_name == "SLB - Play Offs": return "BBL"  
    if league_name == "BNXT League": return "BNXT"
    if league_name == "Champions League - Winners stage": return "CHL"
    if league_name == "Champions League - Play Offs": return "CHL"
    # if league_name == "Champions League": return "CHL"
    if league_name == "Latvian-Estonian League - Play Offs": return "LEL"
    if league_name == "1. Liga": return "1.L"
    if league_name == "1. Liga - Losers stage": return "1L"
    if league_name == "1. Liga - Winners stage": return "1L"
    if league_name == "Korisliiga - Losers stage": return "KOR"
    if league_name == "Korisliiga - Winners stage": return "KOR"
    if league_name == "Korisliiga - Play Offs": return "KOR"
    if league_name == "Eurocup - Play Offs": return "EUR"
    if league_name == "Primera FEB" or league_name == "Primera FEB - Play Offs": return "PF"
    if league_name == "EuroBasket - Qualification - Fourth round": return "EB"
    if league_name == "FIBA Europe Cup - Play Offs": return "EC"
    if league_name == "WCBA Women - Play Offs": return "WCBA"
    if league_name == "B.League" or league_name == "B.League - Play Offs": return "B.L"
    if league_name == "B2.League": return "B2L"
    if league_name == "Division 1": return "D1"
    if league_name == "Korvpalli Meistriliiga - Play Offs" \
        or league_name == "Korvpalli Meistriliiga": return "KOR"
    if league_name == "Philippine Cup": return "PC"
    if league_name == "LNB 2": return "L2"
    if league_name == "Pro Basketball League - Play Offs": return "PBL"
    return league_name


def is_desired_league(game_element):
    try:
        '''
        Starting from this game element, look backwards through the page until you find the first div that 
        has 'tournament__name' in its class name. Use this information to filter out absent leagues
        '''
        league_header = game_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        raw_text = league_header.text.strip()

        league_name, country = get_league_name_and_country(raw_text)
        league_name = league_name.replace('\n', '')
        league_name = league_name.replace('Draw', '')

        desired_leagues = [
                            # 'ACB', #Spain
                            'ACB - Play Offs', #Spain
                            # 'Primera FEB',
                            # 'Primera FEB - Play Offs',
                            # 'SLB', #UK
                            # 'SLB - Play Offs', #UK
                            # 'NBA', #USA
                            # 'NBA - Promotion - Play Offs', #USA
                            # 'NBA - Play Offs', #USA
                            'WNBA', #USA
                            # 'NCAA', #USA
                            # 'NCAA - Play Offs', #USA
                            # 'NIT', #USA
                            # 'NCAA Women', #USA
                            # 'CIBACOPA', #Mexico
                            # 'CIBACOPA - Play Offs', #Mexico
                            'BSN', #Puerto Rico
                            # 'CBA', #China         
                            # 'CBA - Play Offs', #China         
                            # 'WCBA Women', #China
                            # 'WCBA Women - Play Offs', #China
                            # 'VBA', #Vietnam
                            # 'B.League', #Japan
                            # 'B.League - Play Offs', #Japan
                            # 'B2.League', #Japan
                            # 'SBL', #Taiwan
                            # 'TPBL', #Taiwan
                            'TPBL - Play Offs', #Taiwan
                            # 'MPBL', #Philippines
                            'Philippine Cup', #Philippines
                            # 'NBB', #Brazil
                            'NBB - Play Offs', #Brazil
                            # 'Liga A', #Argentina
                            'Liga A - Play Offs', #Argentina
                            # 'Liga A - Play Out', #Argentina
                            # 'Liga Uruguaya', #Uruguay
                            # 'Liga Uruguaya - Play Offs', #Uruguay
                            # 'Liga Uruguaya - Winners stage', #Uruguay
                            # 'Liga Uruguaya - Losers stage', #Uruguay
                            # 'LNB - Apertura', #Paraguay
                            'LBP - Apertura', #Colombia
                            'Libobasquet - First stage', #Bolivia
                            # 'NBL1 East', #Australia
                            # 'NBL1 East Women', #Australia
                            # 'NBL1 North', #Australia
                            # 'NBL1 North Women', #Australia
                            # 'NBL1 South', #Australia
                            # 'NBL1 South Women', #Australia
                            # 'NBL1 Central', #Australia
                            # 'NBL1 Central Women', #Australia
                            # 'NBL1 West', #Australia
                            # 'NBL1 West Women', #Australia
                            
                           
                            # 'BBL', #Germany
                            'BBL - Play Offs', #Germany
                            # 'Pro A', #Germany
                            # 'Pro A - Play Offs', #Germany
                            'LNB', #France, Chile, Dominican Republic
                            # 'LNB - Play-in', #France, Chile
                            'LNB - Play Offs', #France, Chile
                            # 'LNB 2', #Chile
                            # 'Pro B', #France, Germany
                            # 'Pro B - Play - In', #France, Germany
                            # 'Pro B - Play Offs', #France, Germany
                            # 'Lega A', #Italy
                            'Lega A - Play Offs', #Italy
                            # 'Serie A2', #Italy
                            'Pro Basketball League - Play Offs',
                            # 'NB I. A', #Hungary
                            # 'NB I. A - Play Offs', #Hungary
                            # 'NB I. A - Play Out', #Hungary
                            # 'NB I. A - 5th-8th places', #Hungary
                            # 'NB I. A Women', #Hungary
                            # 'NB I. A Women - Play Offs', #Hungary
                            # 'NB I. A Women - Play Out', #Hungary
                            # "DBL", #Netherlands
                            # 'DBL - Play Offs', #Netherlands
                            # 'Eurocup', 
                            # 'Eurocup - Play Offs',
                            # 'ABA League',   
                            # 'ABA League - Play Offs',   
                            # 'BNXT League',
                            # 'Euroleague',
                            # 'Euroleague - Final Four',
                            # 'Euroleague - Play Offs',
                            # 'Champions League',
                            # 'Champions League - Play Offs',
                            # 'Champions League - Winners stage',
                            # 'EuroBasket - Qualification - Fourth round',
                            # 'FIBA Europe Cup - Play Offs',
                            # 'LBL', #Latvia
                            # 'LBL - Play Offs', #Latvia
                            # 'Korvpalli Meistriliiga', #Estonia
                            # 'Korvpalli Meistriliiga - Play Offs', #Estonia
                            # 'Latvian-Estonian League', 
                            # 'Latvian-Estonian League - Play Offs',
                            # 'Korisliiga', #Finland  
                            # 'Korisliiga - Losers stage',
                            # 'Korisliiga - Winners stage',
                            # 'Korisliiga - Play Offs',
                            # 'Basketligaen', #Denmark
                            # 'Basketligaen - Play Offs', #Denmark
                            # 'Basketligaen - Losers stage', #Denmark
                            # 'Basketligaen - Winners stage', #Denmark
                            # 'Basket League', #Greece
                            # 'Basketligan', #Sweden
                            # 'Basketligan - Play Offs', #Sweden
                            # 'Premier League', #Iceland or Saudi Arabia
                            # 'Premier League - Play Offs', #Iceland or Saudi Arabia
                            # 'Premier League Women', #Iceland
                            # 'Premier League Women - Play Offs', #Iceland
                            # 'Super League', #Isreal #Russia
                            # 'Super League - Promotion - Play Offs', #Isreal #Russia
                            # 'Super League - Promotion - Relegation Group', #Isreal #Russia
                            'Super League - Play Offs', #Isreal #Russia
                            # 'VTB United League', #Russia
                            # 'VTB United League - Play Offs', #Russia
                            # 'Superleague', #Georgia
                            # 'Superleague - Play Offs', #Georgia
                            # 'Liga Leumit', #Isreal
                            # 'Liga Leumit - Losers stage', #Isreal
                            # 'Liga Leumit - Winners stage', #Isreal
                            # 'WBL Women', #Isreal women
                            # 'Superliga', #Austria, Venezuela
                            'Superliga - Play Offs', #Austria, Venezuela
                            # 'Superliga - Losers stage', #Austria
                            # 'Superliga - Winners stage', #Austria
                            # 'BLNO', #Norway
                            # 'BLNO - Play Offs', #Norway
                            # 'SB League', #Switzerland
                            # 'SB League - Play Offs', #Switzerland
                            # 'LPB', #Portugal
                            # 'LPB - Play Offs', #Portugal
                            'NBL', #Bulgaria, czech and Austrailia, New zealand
                            # 'NBL - Losers stage',         
                            # 'NBL - Winners stage',
                            # 'NBL - Play Offs',
                            # 'Prva Liga', #Croatia and Macedonia
                            # 'Prva Liga - Play Offs', #Croatia and Macedonia
                            # 'LKL', #Lithuania
                            'LKL - Play Offs', #Lithuania
                            # 'NKL', #Lithuania
                            # 'NKL - Play Offs', #Lithuania
                            # 'NKL - Winners stage', #Lithuania
                            # 'NKL - Losers stage', #Lithuania
                            # 'Premijer liga', #Croatia
                            # 'Premijer liga - Play Offs', #Croatia
                            # 'First League', #Serbia  
                            # 'Division A', #Cyprus
                            # 'Division A - Play Offs', #Cyprus
                            'Division 1', #Lebanon
                            # 'KBL' #Korea
                            # 'KBL - Play Offs' #Korea
                            # 'WKBL Women' #Korea
                            # 'Extraliga', #Slovakia
                            # 'Extraliga - Play Offs', #Slovakia
                            # 'Basket Liga', #Poland
                            'Basket Liga - Play Offs', #Poland
                            # 'Basket Liga - Play in', #Poland
                            # 'Divizia A', #Romania
                            # 'Divizia A - 5th-8th places', #Romania
                            # 'Divizia A - 9th-16th places', #Romania
                            'Divizia A - 13th-16th places', #Romania
                            'Divizia A - Play Offs' #Romania
                            # 'Divizia A - Play Out', #Romania
                            # '1. Liga', #Czech, Poland 
                            # '1. Liga - Losers stage', #Czech
                            # '1. Liga - Winners stage', #Czech
                            # '1. Liga - Play Offs', #Czech, Poland
                            # 'Super Lig' #Turkey
                            # 'Super Lig - Play Offs' #Turkey
                            # 'TBL' #Turkey
                            # 'TBL - Play Offs' #Turkey



                            # 'Euroleague Women - Second stage', 
                            # 'Liga OTP banka', 
                            # 'Premier League Women', 
                            # 'WABA League Women', 
                            # 'EuroCup Women - Play Offs', 
                            # 'Czech Cup', 
                            # 'FIBA Europe Cup - Second stage', 
                            # 'Korisliiga Women', 
                            # 'WBBL Women', 
                            # 'EASL', 
                            # 'NB I. A Women', 'Russian Cup - Play Offs', 'TPBL', 'Commissioners Cup', 
                            # 'Extraliga Women', 'NBA G League', 'Czech Cup Women', 'A2 Women', 
                            
                            # 'Slovenian Cup', 'WNBL Women', 'A1', 'National League', 'VTB United League', 
                            # 'ENBL', 
                        ]
        return (any(league == league_name for league in desired_leagues), get_exact(league_name), country)   
    except NoSuchElementException:
        return (False, "", "")
    

def get_upcoming_games(driver, day = 0):
    driver.get("https://www.flashscore.com/basketball/")
    upcoming = []
    if day == 1:
        next = WebDriverWait(driver, 10).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "button.calendar__navigation--tomorrow"))
        )
        next = driver.find_element(By.CSS_SELECTOR, "button.calendar__navigation--tomorrow")
        driver.execute_script("arguments[0].click();", next)
    sleep(2)

    try:
        # Wait for games to load
        WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "event__match"))
        )
        
        # Re-find elements after waiting to avoid stale references
        games = driver.find_elements(By.CLASS_NAME, "event__match")

        for game in games:
            try:
                is_league, league, country = is_desired_league(game)
                if not is_game_live(game) and is_league:
                    # Re-find elements within each game to avoid stale references
                    teams = WebDriverWait(game, 10).until(
                        EC.presence_of_all_elements_located((By.CLASS_NAME, "event__participant"))
                    )
                    time = WebDriverWait(game, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "event__time"))
                    )
                    game_link = WebDriverWait(game, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "eventRowLink"))
                    ).get_attribute("href")
                    
                    upcoming.append({
                        'league': league,
                        'country': country,
                        'home': teams[0].text,
                        'away': teams[1].text,
                        'time': time.text[:5],
                        'link': game_link
                    })
            except Exception as e:
                print(f"Error processing individual game: {e}")
                continue  # Skip this game and continue with others
    except Exception as e:
        print(f"Error getting upcoming games: {e}")

    return upcoming



def get_quaters_data(driver):
    """Get score in each quater"""
    try:
        count = 0
        while count < 2:
            time.sleep(10)
            wait = WebDriverWait(driver, 5)
            wait.until(EC.presence_of_element_located((By.CLASS_NAME, "smh__part")))
            points = driver.find_elements(By.CLASS_NAME, "smh__part")
            result = [element.text for element in points]
            if len(result[5]) == 0: result[5] = '0' 
            if len(result[11]) == 0: result[11] = '0' 
            if any(len(elem) == 0 for elem in result):
                count += 1
                driver.refresh()
            else: break
        if len(result) < 12:
            result = result + [""] * (12 - len(result))
        return result
    except Exception as e:
        print(f"Error getting quaters scores: {e}")
        return [""] * 12



def get_team_last_matches(driver, element, target_league, section_index):
    target_league = target_league.lower()
    matches = []

    # Click show more only for the specific section we're currently processing
    length = 4 if section_index < 2 else 0
    for _ in range(length):
        try:
            show_more_buttons = driver.find_elements(By.CLASS_NAME, "showMore")
            if len(show_more_buttons) > section_index:
                time.sleep(1)
                driver.execute_script("arguments[0].scrollIntoView(true);", show_more_buttons[section_index])
                time.sleep(1)
                driver.execute_script("arguments[0].click();", show_more_buttons[section_index])
                time.sleep(2)
        except Exception as e:
            print(f'Error clicking show more icon: {e}')

    try:
        cutoff_date = datetime.now() - timedelta(days=365) if target_league == 'NCAA' else \
            datetime.now() - timedelta(days=730)
        
        # Store the current page URL
        original_url = driver.current_url
        
        # For non-H2H sections, process rows normally
        if section_index != 2:
            rows = WebDriverWait(element, 15).until(
                EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__row"))
            )
            
            count = 0
            for row in rows:
                try:
                    # Extract data needed to determine if we'll process this row
                    date_element = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__date"))
                    )
                    date_str = date_element.text
                    match_date = datetime.strptime(date_str, '%d.%m.%y')
                    
                    league_element = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__event"))
                    )
                    league = league_element.text.lower()
                    
                    # Check if we should process this row
                    if not (target_league.startswith(league) and match_date > cutoff_date):
                        continue
                    
                    count += 1
                    if count > 15:
                        break
                    
                    # Get basic match data
                    home_team = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__homeParticipant"))
                    ).text
                    away_team = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__awayParticipant"))
                    ).text
                    score_element = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__result"))
                    )
                    score = score_element.text.strip().split()
                    
                    # Store match data
                    match_data = {
                        'date': date_str,
                        'home': home_team,
                        'away': away_team,
                        'league': league,
                        'home_score': score[0] if len(score) > 0 else '0',
                        'away_score': score[1] if len(score) > 1 else '0',
                        'h_q1': '0',
                        'h_q2': '0',
                        'h_q3': '0',
                        'h_q4': '0',
                        'h_ot': '0', 
                        'a_q1': '0',
                        'a_q2': '0',
                        'a_q3': '0',
                        'a_q4': '0',
                        'a_ot': '0'
                    }
                    
                    matches.append(match_data)
                
                except Exception as e:
                    print(f"Error processing row: {e}")
                    continue
                    
        # For H2H section, use a different approach
        else:
            # Create a second driver for H2H data
            second_driver = None
            try:
                # Get cookies and user agent from the original driver
                cookies = driver.get_cookies()
                user_agent = driver.execute_script("return navigator.userAgent;")
                
                # Set up options for the second driver
                options = webdriver.ChromeOptions()
                options.add_argument(f'user-agent={user_agent}')
                # Add any other options that match your original driver
                options.add_argument('--disable-notifications')
                options.add_argument('--headless') 
                
                # Create a new driver
                second_driver = webdriver.Chrome(options=options)
                
                # Add cookies from the original driver
                second_driver.get(original_url.split('#')[0])
                for cookie in cookies:
                    try:
                        second_driver.add_cookie(cookie)
                    except Exception as e:
                        print(f"Error adding cookie: {e}")
                
            except Exception as e:
                print(f"Error creating second driver: {e}")
                
            # First, collect all match information without clicking
            match_info_list = []
            
            # Get initial rows
            rows = WebDriverWait(element, 15).until(
                EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__row"))
            )
            
            # First pass: collect basic information and store row indexes
            for idx, row in enumerate(rows):
                try:
                    # Extract data needed to determine if we'll process this row
                    date_element = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__date"))
                    )
                    date_str = date_element.text
                    match_date = datetime.strptime(date_str, '%d.%m.%y')
                    
                    league_element = WebDriverWait(row, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "h2h__event"))
                    )
                    league = league_element.text.lower()
                    
                    # Check if we should process this row
                    if target_league.startswith(league) and match_date > cutoff_date:
                        home_team = WebDriverWait(row, 10).until(
                            EC.presence_of_element_located((By.CLASS_NAME, "h2h__homeParticipant"))
                        ).text
                        away_team = WebDriverWait(row, 10).until(
                            EC.presence_of_element_located((By.CLASS_NAME, "h2h__awayParticipant"))
                        ).text
                        score_element = WebDriverWait(row, 10).until(
                            EC.presence_of_element_located((By.CLASS_NAME, "h2h__result"))
                        )
                        score = score_element.text.strip().split()
                        
                        # Store in list
                        match_info_list.append({
                            'row_index': idx,
                            'date': date_str,
                            'home': home_team,
                            'away': away_team,
                            'league': league,
                            'home_score': score[0] if len(score) > 0 else '0',
                            'away_score': score[1] if len(score) > 1 else '0'
                        })
                        
                        # Limit to 15 matches
                        if len(match_info_list) >= 15:
                            break
                
                except Exception as e:
                    print(f"Error collecting match info: {e}")
                    continue
            
            # Second pass: process each match for quarters data
            h2h_count = 0
            for match_info in match_info_list:
                h2h_count += 1
                
                # Initialize match data with default values
                match_data = {
                    'date': match_info['date'],
                    'home': match_info['home'],
                    'away': match_info['away'],
                    'league': match_info['league'],
                    'home_score': match_info['home_score'],
                    'away_score': match_info['away_score'],
                    'h_q1': '0',
                    'h_q2': '0',
                    'h_q3': '0',
                    'h_q4': '0',
                    'h_ot': '0', 
                    'a_q1': '0',
                    'a_q2': '0',
                    'a_q3': '0',
                    'a_q4': '0',
                    'a_ot': '0'
                }
                
                if second_driver:
                    try:
                        # Get fresh rows each time
                        driver.get(original_url)
                        time.sleep(3)
                        
                        # Re-find the H2H section
                        element = WebDriverWait(driver, 15).until(
                            EC.presence_of_element_located((By.XPATH, "//div[contains(@class, 'h2h__section')][" + str(section_index + 1) + "]"))
                        )
                        
                        # Get fresh rows
                        rows = WebDriverWait(element, 15).until(
                            EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__row"))
                        )
                        
                        # Find the row by its index
                        row_index = match_info['row_index']
                        if row_index < len(rows):
                            row = rows[row_index]
                            
                            # Click on the row
                            driver.execute_script("arguments[0].click();", row)
                            time.sleep(3)
                            
                            # Get the current URL
                            match_url = driver.current_url
                            
                            # Use second driver to load the page and get quarters data
                            second_driver.get(match_url)
                            time.sleep(3)
                            
                            # Get quarters data using second driver
                            quaters_score = get_quaters_data(second_driver)
                            
                            # Update match data with quarters
                            match_data['h_q1'] = quaters_score[1] if len(quaters_score) > 1 else '0'
                            match_data['h_q2'] = quaters_score[2] if len(quaters_score) > 2 else '0'
                            match_data['h_q3'] = quaters_score[3] if len(quaters_score) > 3 else '0'
                            match_data['h_q4'] = quaters_score[4] if len(quaters_score) > 4 else '0'
                            match_data['h_ot'] = quaters_score[5] if len(quaters_score) > 5 else '0'
                            match_data['a_q1'] = quaters_score[7] if len(quaters_score) > 7 else '0'
                            match_data['a_q2'] = quaters_score[8] if len(quaters_score) > 8 else '0'
                            match_data['a_q3'] = quaters_score[9] if len(quaters_score) > 9 else '0'
                            match_data['a_q4'] = quaters_score[10] if len(quaters_score) > 10 else '0'
                            match_data['a_ot'] = quaters_score[11] if len(quaters_score) > 11 else '0'
                    
                    except Exception as e:
                        print(f"Error getting quarters data for match {h2h_count}: {e}")
                
                # Add match data to results
                matches.append(match_data)
            
            # Close the second driver
            if second_driver:
                try:
                    second_driver.quit()
                except Exception as e:
                    print(f"Error closing second driver: {e}")
    
    except Exception as e:
        print(f"Error getting matches: {e}")
        # Close second driver on error
        if 'second_driver' in locals() and second_driver:
            try:
                second_driver.quit()
            except:
                pass
    
    return matches


def scrape_h2h_page(driver, url, league):
    try:
        driver.get(url)
        # Handle cookie consent if present
        with suppress(Exception):
            accept_button = WebDriverWait(driver, 5).until(
                EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
            )
            accept_button.click()
            
        # Click H2H tab and wait for it to load
        try:
            h2h_button = WebDriverWait(driver, 10).until(
                EC.element_to_be_clickable((By.CSS_SELECTOR, "a[href='#/h2h'] button"))
            )
            driver.execute_script("arguments[0].click();", h2h_button)
            time.sleep(2)  # Wait for tab to load
        except Exception as e:
            print(f"Error clicking H2H tab: {e}")

        # Get sections with explicit wait
        sections = WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__section"))
        )
        
        results = {
            'home_matches': get_team_last_matches(driver, sections[0], league, 0),
            'away_matches': get_team_last_matches(driver, sections[1], league, 1),
            'h2h_matches': get_team_last_matches(driver, sections[2], league, 2)
        }
        
        return results
        
    except Exception as e:
        print(f"Error in scrape_h2h_page: {e}")
        return {'home_matches': [], 'away_matches': [], 'h2h_matches': []}


def main():
    # 0 for today, 1 for next day games
    day = 1
    driver = setup_driver()
    try:
        # Get today's upcoming games
        upcoming = get_upcoming_games(driver, day)

        number_of_games = len(upcoming)

        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Basketball\NBA2.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Basketball\random2.txt"

        file1_countries = ["USA", "ARGENTINA", "BRAZIL", "VENEZUELA", "CHILE", "CHINA", "TAIWAN", "VIETNAM", "JAPAN", "URUGUAY", "AUSTRALIA",\
                           "NEW ZEALAND", "DOMINICAN REPUBLIC", "SOUTH KOREA", "BOLIVIA", "MEXICO", "PUERTO RICO", "PARAGUAY", "PHILIPPINES", "COLOMBIA"]

        # For each upcoming game, get last 15 scores and H2H
        last_saved = 8  #default value should be 0
        for number, game in enumerate(upcoming):
            if (number+1) > last_saved:
                print(f'{number+1}/{number_of_games}', '\r', end = '')
                #Filter only alphabets
                home_team = game['home']
                away_team = game['away']
                country   = game['country']
                league    = game['league']
                game_time = game['time']

                home_score: str
                away_score: str
                home_h2h_score: str
                away_h2h_score: str

                results = scrape_h2h_page(driver, game['link'], league)

                file = file1 if country in file1_countries  else file2
                output_buffer = StringIO()

                output_buffer.write(f'{home_team}: ')
                for match in results['home_matches']:
                    if home_team == match['home']:
                        home_score = match['home_score']
                    else:
                        home_score = match['away_score']
                    output_buffer.write(home_score+' ')
                output_buffer.write('\n')
 
                output_buffer.write(f'{away_team}: ')
                for match in results['away_matches']:
                    if away_team == match['home']:
                        away_score = match['home_score']
                    else:
                        away_score = match['away_score']
                    output_buffer.write(away_score+' ')
                output_buffer.write('\n')

                if league != 'NCAA':
                    output_buffer.write(f'H2H {len(results["h2h_matches"])}\n')
                    for match in results['h2h_matches']:
                        if home_team == match['home']:
                            home_h2h_score = match['home_score']+' '+match['h_q1']+' '+match['h_q2']+' '+match['h_q3']\
                            +' '+match['h_q4']+' '+match['h_ot']
                            away_h2h_score = match['away_score']+' '+match['a_q1']+' '+match['a_q2']+' '+match['a_q3']\
                            +' '+match['a_q4']+' '+match['a_ot']
                        else:
                            home_h2h_score = match['away_score']+' '+match['a_q1']+' '+match['a_q2']+' '+match['a_q3']\
                            +' '+match['a_q4']+' '+match['a_ot']
                            away_h2h_score = match['home_score']+' '+match['h_q1']+' '+match['h_q2']+' '+match['h_q3']\
                            +' '+match['h_q4']+' '+match['h_ot']
                        output_buffer.write(home_h2h_score+'\n')
                        output_buffer.write(away_h2h_score+'\n')

                output_buffer.write(f'({country}, {league}, {game_time})\n\n')

                with open(file, 'a') as fileObj:
                    fileObj.write(output_buffer.getvalue())
                time.sleep(1)
             
    except Exception as e:
        print(f"Error in main: {e}")  
    finally:
        driver.quit() 

if __name__ == "__main__":
    main()