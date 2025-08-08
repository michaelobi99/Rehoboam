from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.support import expected_conditions as EC
from datetime import datetime, timedelta
import time
from time import sleep
from contextlib import suppress
import re
from threading import Thread


#.....................................................................................................................
#scrape male and female players based on Elo rankings
import requests
from bs4 import BeautifulSoup
import csv
from datetime import datetime
import os
import unicodedata
import re

def clean_text(text):
    """
    Clean text by normalizing Unicode characters and removing problematic symbols
    """
    if not text:
        return text
    
    # Normalize Unicode characters (converts accented characters to their base forms)
    text = unicodedata.normalize('NFKD', text)
    
    # Replace non-breaking spaces and other problematic whitespace characters
    text = text.replace('\u00A0', ' ')  # Non-breaking space
    text = text.replace('\u2009', ' ')  # Thin space
    text = text.replace('\u202F', ' ')  # Narrow no-break space
    text = text.replace('\u2007', ' ')  # Figure space
    text = text.replace('\u2008', ' ')  # Punctuation space
    
    # Replace multiple whitespace characters with single space
    text = re.sub(r'\s+', ' ', text)
    
    # Remove any remaining non-ASCII characters that might cause issues
    # But keep accented characters by only removing control characters
    text = ''.join(char for char in text if unicodedata.category(char)[0] != 'C')
    
    return text.strip()

def scrape_tennis_elo_rankings(urls):
    # URL for Tennis Abstract ATP ELO ratings
    
    # Headers to mimic a browser request
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
    }
    for url in urls:
        try:
            # Send GET request to the URL
            print(f"Fetching data from {url}...")
            response = requests.get(url, headers=headers)
            
            # Explicitly set encoding to handle special characters properly
            response.encoding = 'utf-8'
            
            # Check if the request was successful
            if response.status_code == 200:
                print(f"Successfully retrieved the page: Status code {response.status_code}")
                soup = BeautifulSoup(response.text, 'html.parser')
                table = soup.find_all('table')
                if not table:
                    print("Could not find the rankings table.")
                    return None
                table = table[2]

                # Extract table headers
                headers_list = []
                header_row = table.find('tr')
                if header_row:
                    headers_list = [clean_text(th.text) for th in header_row.find_all('th')]
                    headers_list = [string for string in headers_list if len(string) > 0]
                
                if not headers_list:
                    print("Could not find table headers.")
                    return None
                
                player_rows = table.find_all('tr')[1:]
                
                if not player_rows:
                    print("Could not find player rows.")
                    return None
                
                print(f"Found {len(player_rows)} player rows")
                
                # Prepare data structure
                rankings_data = []
                
                for num, row in enumerate(player_rows):
                    try:
                        # Extract all cells in the row and clean the text
                        cells = [clean_text(x.text) for x in row.find_all(['td', 'th'])]
                        cells = [string for string in cells if len(string) > 0]
                        
                        if len(cells) >= len(headers_list):
                            player_data = {}
                            for i, header in enumerate(headers_list):
                                player_data[header] = cells[i]
                            
                            rankings_data.append(player_data)
                        else:
                            print(f"Row {num+1} has fewer cells ({len(cells)}) than headers ({len(headers_list)})")
                    except Exception as e:
                        print(f"Error extracting data from row: {e}")
                        continue
                
                
                # Save data to CSV
                base = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis"
                atp_file = f"men_elo_rankings.csv"
                wta_file = f"women_elo_rankings.csv"
                csv_filename = os.path.join(base, atp_file) if 'atp' in url else os.path.join(base, wta_file)
                
                # Create directory if it doesn't exist
                os.makedirs(os.path.dirname(csv_filename), exist_ok=True)
                
                with open(csv_filename, 'w', newline='', encoding='utf-8') as csvfile:
                    writer = csv.DictWriter(csvfile, fieldnames=headers_list)
                    writer.writeheader()
                    for player_data in rankings_data:
                        writer.writerow(player_data)
                
                print(f"Data saved to {csv_filename}")
                
            else:
                print(f"Failed to retrieve the page. Status code: {response.status_code}")
                return None
        
        except Exception as e:
            print(f"An error occurred: {e}")
            return None
#........................................................................................................................

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

def determine_surface_from_text(raw_text):
    """
    Determine surface from tournament header text
    """
    raw_text_lower = raw_text.lower()
    if 'hard' in raw_text_lower:
        return 0  # hard
    elif 'clay' in raw_text_lower:
        return 1  # clay
    elif 'grass' in raw_text_lower:
        return 2  # grass
    else:
        return 0  # default to hard

def is_desired_tournament(match_element):
    try:
        tournament_header = match_element.find_element(By.XPATH, "./preceding::div[contains(@class, 'wclLeagueHeader')][1]")
        raw_text = tournament_header.text.strip()
        tournament_name, tournament_type = get_tournament_name_and_type(raw_text)

        # Determine surface from raw text
        surface = determine_surface_from_text(raw_text)
        
        desired_tournaments = [
            'ATP',
            'WTA',
            'CHALLENGER MEN'
            # 'CHALLENGER WOMEN'
            # 'ITF Men',
            # 'ITF Women',
            # 'United Cup',
            # 'Davis Cup',
            # 'Billie Jean King Cup'
        ]
        is_desired: bool = False

        for tournament in desired_tournaments:
            if tournament in tournament_type and 'DOUBLE' not in tournament_type and 'Qualification' not in tournament_name:
                is_desired = True

        return (is_desired, tournament_name, tournament_type, surface)
        
    except NoSuchElementException:
        return (False, "", "", 0)

def get_upcoming_matches(driver, day=0):
    driver.get("https://www.flashscore.com/tennis/")
    upcoming = []
    
    if day == 1:
        next = WebDriverWait(driver, 10).until(
            EC.element_to_be_clickable((By.CSS_SELECTOR, "button[data-day-picker-arrow='next']"))
        )
        next = driver.find_element(By.CSS_SELECTOR, "button[data-day-picker-arrow='next']")
        driver.execute_script("arguments[0].click();", next)
    
    sleep(2)

    try:
        WebDriverWait(driver, 10).until(
            EC.presence_of_all_elements_located((By.CLASS_NAME, "event__match"))
        )
        
        matches = driver.find_elements(By.CLASS_NAME, "event__match")

        for match in matches:
            try:
                is_tournament, tournament_name, tournament_type, surface = is_desired_tournament(match)
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
                        'link': match_link,
                        'surface': surface
                    })
            except Exception as e:
                print(f"Error processing individual match: {e}")
                continue

    except Exception as e:
        print(f"Error getting upcoming matches: {e}")

    return upcoming

def main():
    day = 0 # 0 for today, 1 for next day matches
    
    driver = setup_driver()
    try:
        upcoming = get_upcoming_matches(driver, day)
        number_of_matches = len(upcoming)

        file1 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\ATP_Singles_Matches.txt"
        file2 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\WTA_Singles_Matches.txt"
        file3 = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\Challenger_singles.txt"

        
        last_saved = 0 #Default value is 0
        for number, match in enumerate(upcoming):
            # Convert surface number to string
            surface = 'hard'
            if match['surface'] == 1: 
                surface = 'clay'
            elif match['surface'] == 2: 
                surface = 'grass'

            if (number+1) > last_saved:
                tournament_str = match['tournament']
                tournament_str = tournament_str.split('\n')[0]
                time_str = match['time']
                print(f'{number+1}/{number_of_matches}', '\r', end='')
                player1 = match['player1']
                player2 = match['player2']
                tournament_type = match['type']

                file: str = ""

                if "ATP" in tournament_type:
                    file = file1
                elif "WTA" in tournament_type:
                    file = file2
                else:
                    file = file3

                with open(file, 'a') as fileObj:
                    # Write in the new format: "player1" vs "player2"
                    fileObj.write(f'{player1} vs {player2}\n')
                    fileObj.write(f'{surface}\n')
                    fileObj.write(f'{tournament_type} - {tournament_str} - {time_str}\n\n')
                    
                time.sleep(1)
             
    except Exception as e:
        print(f"Error in main: {e}")
    finally:
        driver.quit()

if __name__ == "__main__":
    atp_elo_site = "https://tennisabstract.com/reports/atp_elo_ratings.html"
    wta_elo_site = "https://tennisabstract.com/reports/wta_elo_ratings.html"
    worker1 = Thread(target=scrape_tennis_elo_rankings, args= ((atp_elo_site, wta_elo_site),))
    worker1.start()
    main()
    worker1.join()