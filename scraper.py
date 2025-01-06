from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException, TimeoutException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from datetime import datetime, timedelta
from contextlib import suppress
from time import sleep


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
    

def is_desired_league(game_element):
    try:
        '''
        Starting from this game element, look backwards through the page until you find the first div that 
        has 'tournament__name' in its class name. Use this information to filter out absent leagues
        '''
        league_header = game_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        desired_leagues = [
                            "ACB", #Spain
                            # "LNB", #France
                            # "BBL", "Pro A", #Germany
                            # "Super Lig", #Turkey
                            # "Premijer liga", #Croatia
                            # "Lega A", #Italy
                            #"NBA", #"NCAA", "WNBA", #USA
                            # "Basket League", #Greece
                            # "NBL", #Aussies
                            # "Superliga", #Austria
                            # "CBA", "WCBA", #China
                            # "Basketligaen", #Denmark
                            # "ABA League", 
                            # "LKL", #Lithuania
                            # "BLNO", #Norway
                            # "Basket Liga", #Poland
                            # "Extraliga", #Slovakia
                            # "KBL", #Korea
                            # "SLB" #England
                            ]
        return any(league in league_header.text for league in desired_leagues)
    except NoSuchElementException:
        return False
    

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
        print(f'Number = {len(games)}')

        for game in games:
            try:
                if not is_game_live(game) and is_desired_league(game):
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
        print(f'Section {section_index} rows: {len(rows)}')
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
    
    return matches

def scrape_h2h_page(driver, url):
    try:
        driver.get(url)
        # Handle cookie consent if present
        with suppress(Exception):
            accept_button = WebDriverWait(driver, 5).until(
                EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
            )
            accept_button.click()
            
        # Click H2H tab
        try:
            h2h_button = WebDriverWait(driver, 10).until(
                EC.element_to_be_clickable((By.CSS_SELECTOR, "a[href='#/h2h'] button"))
            )
            h2h_button.click()
            # Wait for sections to load
            time.sleep(2)
        except Exception as e:
            print(f"Error clicking H2H tab: {e}")

        # Get sections once and process them sequentially
        sections = WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "h2h__section"))
        )
        
        results = {
            'home_matches': get_team_last_matches(driver, sections[0], "ACB", 0),
            'away_matches': get_team_last_matches(driver, sections[1], "ACB", 1),
            'h2h_matches': get_team_last_matches(driver, sections[2], "ACB", 2)
        }
        
        return results
        
    finally:
        driver.quit()

def main():
    # 0 for today, 1 for next day games, and so on
    day = 0
    
    driver = setup_driver()
    try:
        # Get today's upcoming games
        upcoming = get_upcoming_games(driver, day)

        # For each upcoming game, get last 10 scores and H2H
        for game in upcoming:
            print(game['home'])
            print(game['away'])
            print(game['time'])
            print(game['link'])
            driver.get(game['link'])

            results = scrape_h2h_page(driver, game['link'])
            # Print results
            print("\nHome team last 10 matches:")
            for match in results['home_matches']:
                print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")

            print("\nAway team last 10 matches:")
            for match in results['away_matches']:
                print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")

            print("\nH2H matches from last year:")
            for match in results['h2h_matches']:
                print(f"{match['date']} - {match['home']} vs {match['away']}: {match['score']}")
            sleep(10)
            
    finally:
        driver.quit()

if __name__ == "__main__":
    main()