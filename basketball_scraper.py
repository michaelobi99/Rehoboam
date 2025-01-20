from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from time import sleep
from contextlib import suppress


# TO DO
#Use match date to sort and filter duplicates. Also for history capping(1 year)

def setup_driver():
    options = webdriver.ChromeOptions()
    options.add_argument('--disable-notifications')
    options.add_argument('--headless')  # Run in background
    return webdriver.Chrome(options=options)


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
    if league_name == "Premier League": return "PL"
    if league_name == "Premier League Women": return "PRE"
    if league_name == "Prva Liga": return "PL"
    if league_name == "Pro B": return "PB"
    if league_name == "Super League": return "SL"
    if league_name == "Super Lig": return "SL"
    if league_name == "Premijer liga": return "A1"
    if league_name == "First League": return "FL"
    if league_name == "Basket Liga": return "BL"
    if league_name == "Basket League": return "BL"
    if league_name == "Basketligan": return "LIG"
    if league_name == "Liga A": return "LA"
    if league_name == "Superliga": return "ABL"
    if league_name == "NB I. A": return "NBI"
    if league_name == "Lega A": return "LA"
    if league_name == "SLB": return "BBL"  
    if league_name == "BNXT League": return "BNXT"
    # if league_name == "Champions League - Qualification - Winners stage": return "CHL"
    # if league_name == "Champions League": return "CHL"
    if league_name == "1. Liga": return "1L"
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

        desired_leagues = [
                            # 'ACB', #Spain
                            # 'NBA', #USA
                            # 'NCAA', #USA
                            # 'WNBA', #USA
                            # 'CBA', #China         
                            # 'WCBA Women' #China
                            'NBB' #Brasil
                            # 'Liga A', #Argentina
                            # 'BBL', #Germany
                            # 'LNB', #France
                            # 'Pro B', #France
                            # 'Pro A', #Germany
                            # 'Lega A', #Italy
                            # 'NB I. A', #Hungary
                            # 'Eurocup', 
                            # 'ABA League',
                            # 'BNXT League',
                            # 'Euroleague',
                            # 'Champions League'
                            # 'Korisliiga', #Finland  
                            # 'Basketligaen', #Denmark
                            # 'Basket League', #Greece
                            # 'Basketligan', #Sweden
                            # 'Premier League', #Iceland
                            # 'Premier League Women', #Iceland
                            # 'Super League', #Isreal #Russia
                            # 'Superliga', #Austria
                            # 'BLNO', #Norway
                            # 'LPB', #Portugal
                            # 'NBL', #Bulgaria czech and Austrailia         
                            # 'Prva Liga' #Croatia and Macedonia
                            # 'LKL', #Lithuania
                            # 'NKL', #Lithuania
                            # 'Premijer liga', #Croatia
                            # 'First League', #Serbia  
                            # 'KBL', #Korea
                            # 'WKBL Women', #Korea
                            # 'Extraliga', #Slovakia
                            # 'Basket Liga', #Poland
                            # '1. Liga' #Czech
                            # 'Super Lig' #Turkey



                            # 'TBL', #Turkey
                            # 'Latvian-Estonian League', 
                            # 'Euroleague Women - Second stage', 
                            # 'Liga OTP banka', 
                            # 'Premier League Women', 
                            # 'WABA League Women', 
                            # 'NCAA Women', 
                            # 'Division A', 
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


def get_team_last_matches(driver, element, target_league, section_index):
    target_league = target_league.lower()
    matches = []
    
    # Click show more only for the specific section we're currently processing
    if section_index < 2:  # Only for home and away sections, not h2h
        for _ in range(4):
            try:
                show_more_buttons = driver.find_elements(By.CLASS_NAME, "showMore")
                if len(show_more_buttons) > section_index:
                    # Add a small wait before clicking
                    time.sleep(1)
                    driver.execute_script("arguments[0].scrollIntoView(true);", show_more_buttons[section_index])
                    time.sleep(1)
                    driver.execute_script("arguments[0].click();", show_more_buttons[section_index])
            except Exception as e:
                print(f'Error clicking show more icon: {e}')

    try:
        rows = element.find_elements(By.CLASS_NAME, "h2h__row")
        for row in rows:
            try:
                league = row.find_element(By.CLASS_NAME, "h2h__event").text
                league = league.lower()
                if target_league.startswith(league):
                    date = row.find_element(By.CLASS_NAME, "h2h__date").text
                    home_team = row.find_element(By.CLASS_NAME, "h2h__homeParticipant").text
                    away_team = row.find_element(By.CLASS_NAME, "h2h__awayParticipant").text
                    score = row.find_element(By.CLASS_NAME, "h2h__result").text
                    matches.append({
                        'date': date,
                        'home': home_team,
                        'away': away_team,
                        'score': score,
                        'league': league
                    })
            except Exception as e:
                print(f"Error processing match row: {e}")
                continue
                
    except Exception as e:
        print(f"Error getting matches: {e}")
    
    return matches[:15] if section_index < 2 else matches[:5]

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
    day = 0
    
    driver = setup_driver()
    try:
        # Get today's upcoming games
        upcoming = get_upcoming_games(driver, day)

        number_of_games = len(upcoming)

        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\NBA1.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\random1.txt"

        # #clear files
        # with open(file1, 'w'):
        #     pass
        # with open(file2, 'w'):
        #     pass
        # with open(file3, 'w'):
        #     pass

        # For each upcoming game, get last 10 scores and H2H
        for number, game in enumerate(upcoming):
            print(f'{number+1}/{number_of_games}', '\r', end = '')
            #Filter only alphabets
            home_team = game['home']
            away_team = game['away']
            country   = game['country']
            league    = game['league']
            game_time = game['time']

            home_score = []
            away_score = []

            results = scrape_h2h_page(driver, game['link'], league)

            for match in results['home_matches']:
                score = match['score'].strip().split()
                if home_team == match['home']:
                    home_score.append(score[0])
                else:
                    home_score.append(score[1])

            for match in results['away_matches']:
                score = match['score'].strip().split()
                if away_team == match['home']:
                    away_score.append(score[0])
                else:
                    away_score.append(score[1])

            for match in results['h2h_matches']:
                score = match['score'].strip().split()
                if home_team == match['home']:
                    home_score.append(score[0])
                    away_score.append(score[1]) 
                else:
                    home_score.append(score[1])
                    away_score.append(score[0])

            
            file = file2

            with open(file, 'a') as fileObj:
                fileObj.write(f'{home_team}: ')
                fileObj.write(' '.join(str(num) for num in home_score))
                fileObj.write('\n')
                fileObj.write(f'{away_team}: ')
                fileObj.write(' '.join(str(num) for num in away_score))
                fileObj.write('\n')
                fileObj.write(f'({country}, {league}, {game_time})\n\n')
        
            # # Print results
            # print("\nHome team last matches:")
            # for match in results['home_matches']:
            #     print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")

            # print("\nAway team last matches:")
            # for match in results['away_matches']:
            #     print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")

            # print("\nH2H matches:")
            # for match in results['h2h_matches']:
            #     print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")
            
            # Add a small delay between games to avoid overwhelming the server
            time.sleep(3)
             
    except Exception as e:
        print(f"Error in main: {e}")
    finally:
        driver.quit()

if __name__ == "__main__":
    main()