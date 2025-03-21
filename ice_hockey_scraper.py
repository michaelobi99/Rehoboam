from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from time import sleep
from contextlib import suppress

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
    Extracts the league name from header text like 'USA : NHL Standings' or 'EUROPE : Champions Hockey League Standings'
    """
    try:
        parts = header_text.strip().split(':')
        if len(parts) > 1:
            country = parts[0].strip()
            league = parts[1].strip().split('Live Standings' if 'NHL' in parts[1] else 'Standings')[0].strip()
            return (league, country)
        return (header_text.strip(), "")
    except:
        return (header_text.strip(), "")

def get_exact(league_name):
    if league_name == "1st Division": return "D1"
    if league_name == "Extraliga": return "ELH"
    if league_name == "Mestis": return "MES"
    if league_name == "Hockey Allsvenskan": return "HA"
    if league_name == "Metal Ligaen": return "ML"
    if league_name == "GET-ligaen": return "GET"
    return league_name

def is_desired_league(game_element):
    try:
        league_header = game_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        raw_text = league_header.text.strip()

        league_name, country = get_league_name_and_country(raw_text)
        
        desired_leagues = [
            'NHL'  # USA/Canada
            # 'AHL',  # USA/Canada
            # 'KHL',  # Russia
            # 'SHL',  # Sweden
            # 'Liiga',  # Finland
            # 'DEL',  # Germany
            # 'NL',   # Switzerland
            # 'Extraliga',  # Czech
            # 'ICEHL',  # Austria
            # 'Hockey Allsvenskan',  # Sweden
            # 'Mestis',  # Finland
            # 'Champions Hockey League',
            # 'Metal Ligaen',  # Denmark
            # 'GET-ligaen',  # Norway
            # '1st Division'  # Norway
        ]
        
        return (any(league == league_name for league in desired_leagues), get_exact(league_name), country)
    
    except NoSuchElementException:
        return (False, "", "")

def get_upcoming_games(driver, day=0):
    driver.get("https://www.flashscore.com/hockey/")
    upcoming = []
    if day == 1:
        next = WebDriverWait(driver, 10).until(
            EC.presence_of_element_located((By.CSS_SELECTOR, "button.calendar__navigation--tomorrow"))
        )
        next = driver.find_element(By.CSS_SELECTOR, "button.calendar__navigation--tomorrow")
        driver.execute_script("arguments[0].click();", next)
    sleep(2)

    try:
        WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "event__match"))
        )
        
        games = driver.find_elements(By.CLASS_NAME, "event__match")

        for game in games:
            try:
                is_league, league, country = is_desired_league(game)
                if not is_game_live(game) and is_league:
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
                continue
    except Exception as e:
        print(f"Error getting upcoming games: {e}")

    print(len(upcoming))
    return upcoming

def get_team_last_matches(driver, element, target_league, section_index):
    target_league = target_league.lower()
    matches = []
    
    if section_index < 2:
        for _ in range(4):
            try:
                show_more_buttons = driver.find_elements(By.CLASS_NAME, "showMore")
                if len(show_more_buttons) > section_index:
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
                league = row.find_element(By.CLASS_NAME, "h2h__event").text.lower()
                if target_league.startswith(league):
                    date = row.find_element(By.CLASS_NAME, "h2h__date").text
                    home_team = row.find_element(By.CLASS_NAME, "h2h__homeParticipant").text
                    away_team = row.find_element(By.CLASS_NAME, "h2h__awayParticipant").text
                    score = row.find_element(By.CLASS_NAME, "h2h__result").text
                    
                    # Handle overtime/shootout results
                    if 'pen.' in score.lower():
                        score = score.split(' ')[0]  # Take only the final score
                    elif 'ot' in score.lower():
                        score = score.split(' ')[0]  # Take only the final score
                        
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
        with suppress(Exception):
            accept_button = WebDriverWait(driver, 5).until(
                EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
            )
            accept_button.click()
            
        try:
            h2h_button = WebDriverWait(driver, 10).until(
                EC.element_to_be_clickable((By.CSS_SELECTOR, "a[href='#/h2h'] button"))
            )
            driver.execute_script("arguments[0].click();", h2h_button)
            time.sleep(2)
        except Exception as e:
            print(f"Error clicking H2H tab: {e}")

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
    day = 0  # 0 for today, 1 for tomorrow's games
    
    driver = setup_driver()
    try:
        upcoming = get_upcoming_games(driver, day)
        number_of_games = len(upcoming)

        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\NHL1.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\hockey_random1.txt"
        

        for number, game in enumerate(upcoming):
            print(f'{number+1}/{number_of_games}', '\r', end='')
            
            home_team = game['home']
            away_team = game['away']
            country = game['country']
            league = game['league']
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

            file = file2 if league != 'NHL' else file1

            with open(file, 'a') as fileObj:
                fileObj.write(f'{home_team}: ')
                fileObj.write(' '.join(str(num) for num in home_score))
                fileObj.write('\n')
                fileObj.write(f'{away_team}: ')
                fileObj.write(' '.join(str(num) for num in away_score))
                fileObj.write('\n')
                fileObj.write(f'({country}, {league}, {game_time})\n\n')
            time.sleep(3)
             
    except Exception as e:
        print(f"Error in main: {e}")
    finally:
        driver.quit()

if __name__ == "__main__":
    main()