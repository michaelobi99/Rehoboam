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
    

def get_exact(league_name, country):
    country = country.lower()
    if league_name.startswith("NBL1"): return "NBL1"
    if league_name.startswith("Premier League"): return "PL"
    if league_name.startswith("Premier League Women"): return "PRE"
    if league_name.startswith("Prva Liga"): return "PL"
    if league_name.startswith("Pro B"):
        if country == "france": return "PB"
        if country == "germany": return "PROB"
    if league_name.startswith("Pro A"): return "PA"
    if league_name.startswith("Super League"): return "SL"
    if league_name.startswith("Super Lig"): return "SL"
    if league_name.startswith("Liga Leumit"): return "LL"
    if league_name.startswith("Premijer liga"): return "A1"
    if league_name.startswith("First League"): return "FL"
    if league_name.startswith("Basket Liga"): return "BL"
    if league_name.startswith("Basket League"): return "BL"
    if league_name.startswith("SB League"): return "SBL"
    if league_name.startswith("Basketligan"): return "LIG"
    if league_name.startswith("Liga A"): return "LA"
    if league_name.startswith("Liga Uruguaya"): return "LC"
    if league_name.startswith("Superleague"): return "SL"
    if league_name.startswith("Superliga"):
        if country == "austria": return "ABL"
        else: return "SL"
    if league_name.startswith("NB I. A"): return "NBI"
    if league_name.startswith("NB I. A Women"): return "DIV"
    if league_name.startswith("Lega A"): return "LA"
    if league_name.startswith("Serie A2"): return "A2"
    if league_name.startswith("SLB"): return "BBL"  
    if league_name.startswith("BNXT League"): return "BNXT"
    if league_name.startswith("Champions League"): return "CHL"
    if league_name.startswith("Latvian-Estonian League"): return "LEL"
    if league_name.startswith("1. Liga"):
        if country == "czech republic": return "1L"
        if country == "poland": return "1.L"
    if league_name.startswith("Korisliiga"): return "KOR"
    if league_name.startswith("Eurocup"): return "EUR"
    if league_name.startswith("Primera FEB"): return "PF"
    if league_name.startswith("EuroBasket"): return "EB"
    if league_name.startswith("FIBA Europe Cup"): return "EC"
    if league_name.startswith("WCBA Women"): return "WCBA"
    if league_name.startswith("B.League"): return "B.L"
    if league_name.startswith("B2.League"): return "B2L"
    if league_name.startswith("Division 1"): return "D1"
    if league_name.startswith("Korvpalli Meistriliiga"): return "KOR"
    if league_name.startswith("Philippine Cup"): return "PC"
    if league_name.startswith("LNB 2"): return "L2"
    if league_name.startswith("Pro Basketball League"): return "PBL"
    if league_name.startswith("Asia Champions League"): return "BCL"
    if league_name.startswith("EuroBasket"): return "EB"
    if league_name == "NBA Las Vegas Summer League": return "LVSL"
    return league_name


def is_desired_league(game_element):
    try:
        '''
        Starting from this game element, look backwards through the page until you find the first div that 
        has 'tournament__name' in its class name. Use this information to filter out absent leagues
        '''
        league_header = game_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'headerLeague__wrapper')][1]")
        raw_text = league_header.text.strip()

        league_name, country = get_league_name_and_country(raw_text)
        league_name = league_name.replace('\n', '')
        league_name = league_name.replace('Draw', '')

        desired_leagues = [
                            # 'BAL', #Africa,
                            # 'AfroBasket', #Africa
                            # 'BAL - Qualification - Play Offs', #Africa,
                            # 'BAL - Play Offs', #Africa,
                            # 'Asia Champions League', #what it said
                            # 'Asia Champions League - Play Offs', #what it said
                            # 'ACB', #Spain
                            # 'ACB - Play Offs', #Spain
                            # 'Primera FEB',
                            # 'Primera FEB - Play Offs',
                            # 'SLB', #UK
                            # 'SLB - Play Offs', #UK
                            # 'NBA', #USA
                            # 'NBA - Promotion - Play Offs', #USA
                            # 'NBA - Play Offs', #USA
                            # 'NBA Las Vegas Summer League', #USA
                            # 'WNBA', #USA
                            # 'WNBA - Play Offs', #USA
                            # 'NCAA', #USA
                            # 'NCAA - Play Offs', #USA
                            # 'NIT', #USA
                            # 'NCAA Women', #USA
                            # 'CEBL', #Canada
                            # 'CIBACOPA', #Mexico
                            # 'CIBACOPA - Play Offs', #Mexico
                            # 'LNBP', #Mexico
                            # 'BSN', #Puerto Rico
                            # 'BSN - Play Offs' #Puerto Rico
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
                            # 'TPBL - Play Offs', #Taiwan
                            # 'MPBL' #Philippines
                            # 'Philippine Cup', #Philippines
                            # 'NBB', #Brazil
                            # 'NBB - Play Offs', #Brazil
                            # 'Liga A', #Argentina
                            # 'Liga A - Play Offs', #Argentina
                            # 'Liga A - Play Out', #Argentina
                            # 'Liga Uruguaya', #Uruguay
                            # 'Liga Uruguaya - Play Offs', #Uruguay
                            # 'Liga Uruguaya - Winners stage', #Uruguay
                            # 'Liga Uruguaya - Losers stage', #Uruguay
                            # 'LNB - Apertura', #Paraguay
                            # 'LNB - Clausura', #Paraguay
                            # 'LBP - Apertura', #Colombia
                            # 'LBP - Apertura - Play Offs', #Colombia
                            # 'Libobasquet - First stage', #Bolivia
                            # 'Libobasquet - Play Offs', #Bolivia
                            # 'NBL1 East', #Australia
                            # 'NBL1 East Women', #Australia
                            # 'NBL1 North', #Australia
                            # 'NBL1 North Women', #Australia
                            # 'NBL1 South', #Australia
                            # 'NBL1 South Women', #Australia
                            # 'NBL1 Central', #Australia
                            # 'NBL1 Central Women', #Australia
                            # 'NBL1 West' #Australia
                            # 'NBL1 West Women' #Australia
                            
                           
                            #'EuroBasket',
                            # 'EuroBasket - Play Offs',
                            # 'BBL', #Germany
                            # 'BBL - Play Offs', #Germany
                            # 'Pro A', #Germany
                            # 'Pro A - Play Offs', #Germany
                            # 'LNB', #France, Chile, Dominican Republic
                            # 'LNB - Winners stage', #Dominican Republic
                            # 'LNB - Play-in', #France, Chile
                            # 'LNB - Play Offs', #France, Chile, Dominican Republic
                            # 'LNB 2', #Chile
                            # 'Pro B', #France, Germany
                            # 'Pro B - Play - In', #France, Germany
                            # 'Pro B - Play Offs', #France, Germany
                            # 'Lega A', #Italy
                            # 'Lega A - Play Offs', #Italy
                            # 'Serie A2', #Italy
                            # 'Serie A2 - Play Offs', #Italy
                            # 'Pro Basketball League - Play Offs',
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
                            # 'LBBL', #Luxembourg
                            # 'Korvpalli Meistriliiga', #Estonia
                            # 'Korvpalli Meistriliiga - Play Offs', #Estonia
                            'Latvian-Estonian League'
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
                            # 'Super League - Play Offs', #Isreal #Russia
                            # 'VTB United League', #Russia
                            # 'VTB United League - Play Offs', #Russia
                            # 'Superleague', #Georgia
                            # 'Superleague - Play Offs', #Georgia
                            # 'Liga Leumit', #Isreal
                            # 'Liga Leumit - Losers stage', #Isreal
                            # 'Liga Leumit - Winners stage', #Isreal
                            # 'WBL Women', #Isreal women
                            # 'Superliga', #Austria, Venezuela
                            # 'Superliga - Final Group', #Austria, Venezuela
                            # 'Superliga - Play Offs', #Austria, Venezuela
                            # 'Superliga - Losers stage', #Austria
                            # 'Superliga - Winners stage', #Austria
                            # 'BLNO', #Norway
                            # 'BLNO - Play Offs', #Norway
                            # 'SB League', #Switzerland
                            # 'SB League - Play Offs', #Switzerland
                            # 'LPB', #Portugal
                            # 'LPB - Play Offs', #Portugal
                            # 'NBL', #Bulgaria, czech and Austrailia, New zealand, Singapore
                            # 'NBL - Losers stage',         
                            # 'NBL - Winners stage',
                            # 'NBL - Play Offs',
                            # 'Prva Liga', #Croatia and Macedonia
                            # 'Prva Liga - Play Offs', #Croatia and Macedonia
                            # 'LKL', #Lithuania
                            # 'LKL - Play Offs', #Lithuania
                            # 'NKL', #Lithuania
                            # 'NKL - Play Offs', #Lithuania
                            # 'NKL - Winners stage', #Lithuania
                            # 'NKL - Losers stage', #Lithuania
                            # 'Premijer liga', #Croatia
                            # 'Premijer liga - Play Offs', #Croatia
                            # 'First League', #Serbia  
                            # 'Division A', #Cyprus
                            # 'Division A - Play Offs', #Cyprus
                            # 'Division 1', #Lebanon
                            # 'Division 1 - Relegation - Play Offs', #Lebanon
                            # 'Division 1 - Play Offs' #Lebanon
                            # 'KBL', #Korea
                            # 'KBL - Play Offs' #Korea
                            # 'WKBL Women' #Korea
                            # 'IBL', #Indonesia
                            # 'IBL - Play Offs', #Indonesia
                            # 'Extraliga', #Slovakia
                            # 'Extraliga - Play Offs', #Slovakia
                            # 'Basket Liga', #Poland
                            # 'Basket Liga - Play Offs' #Poland
                            # 'Basket Liga - Play in', #Poland
                            # 'Liga OTP banka', #Slovenia
                            # 'Liga OTP banka - Play Offs', #Slovenia
                            # 'Divizia A' #Romania
                            # 'Divizia A - 5th-8th places', #Romania
                            # 'Divizia A - 9th-16th places', #Romania
                            # 'Divizia A - 13th-16th places', #Romania
                            # 'Divizia A - Play Offs', #Romania
                            # 'Divizia A - Play Out', #Romania
                            # '1. Liga', #Czech, Poland 
                            # '1. Liga - Losers stage', #Czech
                            # '1. Liga - Winners stage', #Czech
                            # '1. Liga - Play Offs', #Czech, Poland
                            # 'Super Lig' #Turkey
                            # 'Super Lig - Play Offs', #Turkey
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
        return (any(league_name == league for league in desired_leagues), get_exact(league_name, country), country)   
    except NoSuchElementException:
        return (False, "", "")
    

def get_upcoming_games(driver, day = 0):
    driver.get("https://www.flashscore.com/basketball/")

    with suppress(Exception):
        accept_button = WebDriverWait(driver, 5).until(
            EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
        )
        accept_button.click()
    
    upcoming = []

    if day > 0:
        for _ in range(day):
            next = WebDriverWait(driver, 10).until(
                EC.element_to_be_clickable((By.CSS_SELECTOR, "button[data-day-picker-arrow='next']"))
            )
            next = driver.find_element(By.CSS_SELECTOR, "button[data-day-picker-arrow='next']")
            driver.execute_script("arguments[0].click();", next)
            sleep(3)

    try:
        # Wait for games to load
        WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "event__match"))
        )
        
        games = driver.find_elements(By.CLASS_NAME, "event__match")
        for game in games:
            try:
                is_league, league, country = is_desired_league(game)
                if is_league:
                    teams = game.find_elements(By.CLASS_NAME, "event__participant")
                    time = game.find_element(By.CLASS_NAME, "event__time")
                    game_link = game.find_element(By.CLASS_NAME, "eventRowLink").get_attribute("href")
                    
                    upcoming.append({
                        'league': league,
                        'country': country,
                        'home': teams[0].text,
                        'away': teams[1].text,
                        'time': time.text[:5],
                        'link': game_link
                    })
            except Exception as e:
                continue 
    except Exception as e:
        print(f"Error getting upcoming games: {e}")

    return upcoming


def get_quaters_data(driver):
    try:
        count = 0
        while count < 2:
            time.sleep(5)
            wait = WebDriverWait(driver, 5)
            wait.until(EC.presence_of_element_located((By.CLASS_NAME, "smh__part")))
            points = driver.find_elements(By.CLASS_NAME, "smh__part")
            result = [element.text for element in points]
            if len(result) > 11:
                if len(result[5]) == 0: result[5] = '0' 
                if len(result[11]) == 0: result[11] = '0' 
            if any(len(elem) == 0 for elem in result):
                count += 1
                driver.refresh()
            else: 
                break
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
    length = 4 if section_index < 2 else 1
    for _ in range(length):
        try:            
            show_more_button = element.find_element(By.CLASS_NAME, "wclButtonLink--h2h")
            time.sleep(1)
            driver.execute_script("arguments[0].scrollIntoView(true);", show_more_button)
            time.sleep(1)
            driver.execute_script("arguments[0].click();", show_more_button)
            time.sleep(1)
        except Exception as e:
            print(f'Error clicking show more icon: {e}')
    

    try:
        cutoff_date = datetime.now() - timedelta(days=365) if target_league == 'NCAA' else \
            datetime.now() - timedelta(days=730)
        
        rows = WebDriverWait(element, 15).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__row"))
        )
        
        count = 0
        for row in rows:
            try:
                count += 1
                if section_index < 2:
                    if count > 10:
                        break
                else:
                    if count > 5:
                        break
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
                
                # Get basic match data
                sleep(2)
                match_link = row.get_attribute("href")
                home_team = row.find_element(By.CLASS_NAME, "h2h__homeParticipant").text
                away_team = row.find_element(By.CLASS_NAME, "h2h__awayParticipant").text
                score_element = row.find_element(By.CLASS_NAME, "h2h__result")
                score = score_element.text.strip().split()
                
                # Store match data
                match_data = {
                    'link': match_link,
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


        if section_index == 2:
            original_window = driver.current_window_handle
            for idx in range(len(matches)):
                try:
                    link = matches[idx]['link']
                    driver.execute_script("window.open('');")
                    driver.switch_to.window(driver.window_handles[-1])
                    driver.get(link)
                    quaters_score = get_quaters_data(driver)
                    # Update match data with quarters
                    matches[idx]['h_q1'] = quaters_score[1] if len(quaters_score[1]) > 0 else '0'
                    matches[idx]['h_q2'] = quaters_score[2] if len(quaters_score[2]) > 0 else '0'
                    matches[idx]['h_q3'] = quaters_score[3] if len(quaters_score[3]) > 0 else '0'
                    matches[idx]['h_q4'] = quaters_score[4] if len(quaters_score[4]) > 0 else '0'
                    matches[idx]['h_ot'] = quaters_score[5] if len(quaters_score[5]) > 0 else '0'
                    matches[idx]['a_q1'] = quaters_score[7] if len(quaters_score[7]) > 0 else '0'
                    matches[idx]['a_q2'] = quaters_score[8] if len(quaters_score[8]) > 0 else '0'
                    matches[idx]['a_q3'] = quaters_score[9] if len(quaters_score[9]) > 0 else '0'
                    matches[idx]['a_q4'] = quaters_score[10] if len(quaters_score[10]) > 0 else '0'
                    matches[idx]['a_ot'] = quaters_score[11] if len(quaters_score[11]) > 0 else '0'
                    driver.close()
                except Exception as e:
                    print(f"Error clicking quaters data link: {e}")
                    continue
                finally:
                    driver.switch_to.window(original_window)
                    
    except Exception as e:
        print(f"Error getting matches: {e}")
    
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
            sleep(2)
            tab_buttons = driver.find_elements(By.CSS_SELECTOR, "div.detailOver > div > a")
            h2h_button = tab_buttons[-2]
            driver.execute_script("arguments[0].click();", h2h_button)
            sleep(2)  # Wait for tab to load
        except Exception as e:
            print(f"Error clicking H2H tab: {e}")
            return

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

        file = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Basketball\NBA2.txt"
        # file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Basketball\random2.txt"

        # file1_countries = ["USA", "ARGENTINA", "BRAZIL", "VENEZUELA", "CHILE", "CHINA", "TAIWAN", "VIETNAM", "JAPAN", "URUGUAY", "AUSTRALIA",\
        #                    "NEW ZEALAND", "DOMINICAN REPUBLIC", "SOUTH KOREA", "BOLIVIA", "MEXICO", "PUERTO RICO", "PARAGUAY", "PHILIPPINES", "COLOMBIA"]

        # For each upcoming game, get last 15 scores and H2H
        last_saved = 0 #default value should be 0
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

                # file = file1 if country in file1_countries  else file2

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
                            +' '+match['h_q4']+' '+match['h_ot'] + ' 1'
                            away_h2h_score = match['away_score']+' '+match['a_q1']+' '+match['a_q2']+' '+match['a_q3']\
                            +' '+match['a_q4']+' '+match['a_ot'] + ' 2'
                        else:
                            home_h2h_score = match['away_score']+' '+match['a_q1']+' '+match['a_q2']+' '+match['a_q3']\
                            +' '+match['a_q4']+' '+match['a_ot'] + ' 2'
                            away_h2h_score = match['home_score']+' '+match['h_q1']+' '+match['h_q2']+' '+match['h_q3']\
                            +' '+match['h_q4']+' '+match['h_ot'] + ' 1'
                        output_buffer.write(home_h2h_score+'\n')
                        output_buffer.write(away_h2h_score+'\n')

                output_buffer.write(f'({country}, {league}, {game_time})\n\n')

                with open(file, 'a') as fileObj:
                    fileObj.write(output_buffer.getvalue())
             
    except Exception as e:
        print(f"Error in main: {e}")  
    finally:
        driver.quit() 

if __name__ == "__main__":
    main()