from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from contextlib import suppress


def setup_driver():
    options = webdriver.ChromeOptions()
    options.add_argument('--disable-notifications')
    #options.add_argument('--headless')  # Run in background
    return webdriver.Chrome(options=options)


def is_game_live(game_element):
    try:
        game_element.find_element(By.CLASS_NAME, "event__stage")
        return True
    except NoSuchElementException:
        return False
    

def get_clean_league_name(header_text):
    """
    Extracts the league name from header text like 'USA : NCAA Standings' or 'EUROPE : Eurocup Standings'
    Returns just the league name (e.g., 'NCAA' or 'Eurocup')
    """
    try:
        parts = header_text.strip().split(':')
        if len(parts) > 1:
            league = parts[1].strip().split('Standings')[0].strip()
            return league
        return header_text.strip()
    except:
        return header_text.strip()
    

def is_desired_league(game_element):
    try:
        '''
        Starting from this game element, look backwards through the page until you find the first div that 
        has 'tournament__name' in its class name. Use this information to filter out absent leagues
        '''
        league_header = game_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        raw_text = league_header.text.strip()

        league_name = get_clean_league_name(raw_text)

        desired_leagues = [
                            'ACB', 
                            # 'NBA', 
                            # 'CBA'
                            # 'NCAA', 
                            # 'Eurocup', 
                            # 'Korisliiga', 
                            # 'NBL', 
                            # 'KBL', 
                            # 'Extraliga', 
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
                            # 'WKBL Women', 
                            # 'WBBL Women', 
                            # 'Superliga', 
                            # 'Draw', 
                            # 'EASL', 
                            # 'BLNO', 
                            # 'NB I. A Women', 'Russian Cup - Play Offs', 'TPBL', 'Commissioners Cup', 
                            # 'Extraliga Women', 'NBA G League', 'Czech Cup Women', 'A2 Women', 
                            # 'Champions League - Qualification - Winners stage', 'WCBA Women', 'Liga A', 
                            # 'Slovenian Cup', 'WNBL Women', 'A1', 'National League', 'VTB United League', 
                            # 'ENBL', 
                            ]
        return (any(league in league_name for league in desired_leagues), league_name)
    except NoSuchElementException:
        return (False, "")
    

def get_upcoming_games(driver, day = 0):
    driver.get("https://www.flashscore.com/basketball/")
    upcoming = []
    try:
        for _ in range(day):
            next = WebDriverWait(driver, 10).until(
                EC.presence_of_element_located((By.CSS_SELECTOR, "button.calendar__navigation--tomorrow"))
            )
            next.click()
    except BaseException as e:
        print(f"Error locating next day button: {e}")

    try:
        # Wait for games to load
        WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "event__match"))
        )
        
        # Re-find elements after waiting to avoid stale references
        games = driver.find_elements(By.CLASS_NAME, "event__match")

        for game in games:
            try:
                is_league, league = is_desired_league(game)
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

    print(f'Number = {len(upcoming)}')
    return upcoming


def get_team_last_matches(driver, element, target_league, section_index):
    matches = []
    
    # Click show more only for the specific section we're currently processing
    if section_index < 2:  # Only for home and away sections, not h2h
        for _ in range(3):
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
                if league == target_league:
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
    
    return matches[:10] if section_index < 2 else matches[:5]

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
    # 0 for today, 1 for next day games, and so on
    day = 0
    
    driver = setup_driver()
    try:
        # Get today's upcoming games
        upcoming = get_upcoming_games(driver, day)
        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\NBA1.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\NCAA1.txt"
        file3 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\random1.txt"

        #clear files
        with open(file1, 'w'):
            pass
        with open(file2, 'w'):
            pass
        with open(file3, 'w'):
            pass

        # For each upcoming game, get last 10 scores and H2H
        for game in upcoming:
            home_team, away_team = game['home'], game['away']
            results = scrape_h2h_page(driver, game['link'], game['league'])

            league = game['league']
            game_time =  game['time']
            home_score = []
            away_score = []

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

            file = file1 if league == 'NBA' or league == 'WNBA' else file2 if league == 'NCAA' else file3

            with open(file, 'a') as fileObj:
                fileObj.write(f'{home_team} ')
                fileObj.write(' '.join(str(num) for num in home_score))
                fileObj.write('\n')
                fileObj.write(f'{away_team} ')
                fileObj.write(' '.join(str(num) for num in away_score))
                fileObj.write('\n')
                fileObj.write(f'{game_time}\n\n')
        


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