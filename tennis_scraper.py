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

def is_match_live(match_element):
    try:
        match_element.find_element(By.CLASS_NAME, "event__stage")
        return True
    except NoSuchElementException:
        return False

def get_tournament_name_and_type(header_text):
    """
    Extracts the tournament name and type (e.g., 'ATP', 'WTA', 'ITF') from header text
    """
    try:
        parts = header_text.strip().split(':')
        if len(parts) > 1:
            tournament_type = parts[0].strip()
            tournament_name = parts[1].strip().split('Standings')[0].strip()
            return (tournament_name, tournament_type)
        return (header_text.strip(), "")
    except:
        return (header_text.strip(), "")

def is_desired_tournament(match_element):
    try:
        tournament_header = match_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        raw_text = tournament_header.text.strip()
        tournament_name, tournament_type = get_tournament_name_and_type(raw_text)

        desired_tournaments = [
            'ATP',
            'WTA',
            'Challenger',
            'ITF Men',
            'ITF Women',
            'United Cup',
            'Davis Cup',
            'Billie Jean King Cup'
        ]
        
        return (any(tourney in tournament_type for tourney in desired_tournaments), 
                tournament_name, tournament_type)
                
    except NoSuchElementException:
        return (False, "", "")

def get_upcoming_matches(driver, day=0):
    driver.get("https://www.flashscore.com/tennis/")
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
        
        matches = driver.find_elements(By.CLASS_NAME, "event__match")

        for match in matches:
            try:
                is_tournament, tournament_name, tournament_type = is_desired_tournament(match)
                if not is_match_live(match) and is_tournament:
                    players = WebDriverWait(match, 10).until(
                        EC.presence_of_all_elements_located((By.CLASS_NAME, "event__participant"))
                    )
                    time = WebDriverWait(match, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "event__time"))
                    )
                    match_link = WebDriverWait(match, 10).until(
                        EC.presence_of_element_located((By.CLASS_NAME, "eventRowLink"))
                    ).get_attribute("href")
                    
                    upcoming.append({
                        'tournament': tournament_name,
                        'type': tournament_type,
                        'player1': players[0].text,
                        'player2': players[1].text,
                        'time': time.text[:5],
                        'link': match_link
                    })
            except Exception as e:
                print(f"Error processing individual match: {e}")
                continue

    except Exception as e:
        print(f"Error getting upcoming matches: {e}")

    return upcoming

def get_player_last_matches(driver, element, tournament_type, section_index):
    matches = []
    
    if section_index < 2:  # Only for player1 and player2 sections, not h2h
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
        cutoff_date = datetime.now() - timedelta(days=365)

        for row in rows:
            try:
                date_str = row.find_element(By.CLASS_NAME, "h2h__date").text
                match_date = datetime.strptime(date_str, '%d.%m.%y')
                
                if match_date > cutoff_date:
                    tournament = row.find_element(By.CLASS_NAME, "h2h__event").text
                    player1 = row.find_element(By.CLASS_NAME, "h2h__homeParticipant").text
                    player2 = row.find_element(By.CLASS_NAME, "h2h__awayParticipant").text
                    score = row.find_element(By.CLASS_NAME, "h2h__result").text
                    matches.append({
                        'date': date_str,
                        'player1': player1,
                        'player2': player2,
                        'score': score,
                        'tournament': tournament
                    })
            except Exception as e:
                print(f"Error processing match row: {e}")
                continue
                
    except Exception as e:
        print(f"Error getting matches: {e}")
    
    return matches[:15] if section_index < 2 else matches[:5]

def scrape_h2h_page(driver, url, tournament_type):
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
            'player1_matches': get_player_last_matches(driver, sections[0], tournament_type, 0),
            'player2_matches': get_player_last_matches(driver, sections[1], tournament_type, 1),
            'h2h_matches': get_player_last_matches(driver, sections[2], tournament_type, 2)
        }
        
        return results
        
    except Exception as e:
        print(f"Error in scrape_h2h_page: {e}")
        return {'player1_matches': [], 'player2_matches': [], 'h2h_matches': []}

def parse_tennis_score(score_text):
    """
    Parse tennis score and return a simplified version
    Example: '6-4, 6-2' -> '2-0' (sets won)
    """
    try:
        sets = score_text.split(', ')
        player1_sets = 0
        player2_sets = 0
        
        for set_score in sets:
            if 'Retired' in set_score or 'Walkover' in set_score:
                return 'ret'
                
            games = set_score.split('-')
            if len(games) == 2:
                game1 = int(games[0])
                game2 = int(games[1])
                if game1 > game2:
                    player1_sets += 1
                elif game2 > game1:
                    player2_sets += 1
                    
        return f'{player1_sets}-{player2_sets}'
    except:
        return 'error'

def main():
    day = 1  # 0 for today, 1 for next day matches
    
    driver = setup_driver()
    try:
        upcoming = get_upcoming_matches(driver, day)
        number_of_matches = len(upcoming)

        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\ATP1.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\Tennis1.txt"

        file1_types = ["ATP", "WTA"]  # Main tours in file1
        
        last_saved = -1
        for number, match in enumerate(upcoming):
            if number > last_saved:
                print(f'{number+1}/{number_of_matches}', '\r', end='')
                
                player1 = match['player1']
                player2 = match['player2']
                tournament_type = match['type']
                tournament_name = match['tournament']
                match_time = match['time']

                player1_scores = []
                player2_scores = []

                results = scrape_h2h_page(driver, match['link'], tournament_type)

                # Process player1's recent matches
                for recent_match in results['player1_matches']:
                    score = parse_tennis_score(recent_match['score'])
                    if player1 == recent_match['player1']:
                        player1_scores.append(score)
                    else:
                        # Reverse score if player1 was player2 in that match
                        sets = score.split('-')
                        player1_scores.append(f'{sets[1]}-{sets[0]}' if len(sets) == 2 else score)

                # Process player2's recent matches
                for recent_match in results['player2_matches']:
                    score = parse_tennis_score(recent_match['score'])
                    if player2 == recent_match['player1']:
                        player2_scores.append(score)
                    else:
                        sets = score.split('-')
                        player2_scores.append(f'{sets[1]}-{sets[0]}' if len(sets) == 2 else score)

                # Add separator and process H2H matches
                player1_scores.append(':')
                player2_scores.append(':')
                
                for h2h_match in results['h2h_matches']:
                    score = parse_tennis_score(h2h_match['score'])
                    if player1 == h2h_match['player1']:
                        player1_scores.append(score)
                        sets = score.split('-')
                        player2_scores.append(f'{sets[1]}-{sets[0]}' if len(sets) == 2 else score)
                    else:
                        sets = score.split('-')
                        player1_scores.append(f'{sets[1]}-{sets[0]}' if len(sets) == 2 else score)
                        player2_scores.append(score)

                file = file1 if any(t in tournament_type for t in file1_types) else file2

                with open(file, 'a') as fileObj:
                    fileObj.write(f'{player1}: ')
                    fileObj.write(' '.join(str(score) for score in player1_scores))
                    fileObj.write('\n')
                    fileObj.write(f'{player2}: ')
                    fileObj.write(' '.join(str(score) for score in player2_scores))
                    fileObj.write('\n')
                    fileObj.write(f'({tournament_type}, {tournament_name}, {match_time})\n\n')
                    
                time.sleep(3)
             
    except Exception as e:
        print(f"Error in main: {e}")
    finally:
        driver.quit()

if __name__ == "__main__":
    main()