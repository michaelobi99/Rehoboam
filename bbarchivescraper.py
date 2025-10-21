import time
import os
import csv
import pandas as pd
from datetime import datetime
from typing import Dict, List
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.chrome.options import Options
from selenium.common.exceptions import TimeoutException, NoSuchElementException
from contextlib import suppress
from threading import Thread
from datetime import datetime


class FlashscoreBasketballScraper:
    def __init__(self, headless: bool = True, delay: float = 2.0):
        self.base_url = "https://www.flashscore.com"
        self.delay = delay
        self.driver = None
        self.stats_driver = None
        self.headless = headless
        
        # Headers to mimic real browser
        self.headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
            'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
            'Accept-Language': 'en-US,en;q=0.5',
            'Accept-Encoding': 'gzip, deflate',
            'Connection': 'keep-alive',
            'Upgrade-Insecure-Requests': '1',
        }
        
    def setup_driver(self) -> webdriver.Chrome:
        chrome_options = Options()
        if self.headless:
            chrome_options.add_argument("--headless")
        chrome_options.add_argument("--no-sandbox")
        chrome_options.add_argument("--disable-dev-shm-usage")
        chrome_options.add_argument("--disable-blink-features=AutomationControlled")
        chrome_options.add_experimental_option("excludeSwitches", ["enable-automation"])
        chrome_options.add_experimental_option('useAutomationExtension', False)
        chrome_options.add_argument(f"user-agent={self.headers['User-Agent']}")
        
        driver = webdriver.Chrome(options=chrome_options)
        driver.execute_script("Object.defineProperty(navigator, 'webdriver', {get: () => undefined})")
        return driver
    


    def get_seasons_list(self, country: str, league: str):
        country = country.lower()
        league = league.lower()
        archive_url = f"{self.base_url}/basketball/{country}/{league}/archive/"
        
        if not self.driver:
            self.driver = self.setup_driver()
            
        try:
            self.driver.get(archive_url) 
            with suppress(Exception):
                accept_button = WebDriverWait(self.driver, 5).until(
                    EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
                )
                accept_button.click()

            time.sleep(3)
            
            season_elements = self.driver.find_elements(By.CSS_SELECTOR, "div.archive__season > a.archive__text")
            seasons = []
            season_links = []

            if (len(season_elements) > 0):
                for element in season_elements:
                    seasons.append(element.text)
                    season_links.append(element.get_attribute("href"))
                return seasons, season_links
                
            else:
                print("Error: Season list is empty!")
                    
        except Exception as e:
            print(f"Error getting seasons list: {e}")
            return ([], [])
        

    def get_season_matches(self, link: str, saved_count: int):
        link = link + "results/"
        print(link)
        self.driver.get(link)
        time.sleep(3)
        while True:
            try:
                show_more_button = WebDriverWait(self.driver, 10).until(
                    EC.element_to_be_clickable((By.CLASS_NAME, "wclButtonLink"))
                )
                self.driver.execute_script("arguments[0].scrollIntoView();", show_more_button)
                time.sleep(0.5) 
                self.driver.execute_script("arguments[0].click();", show_more_button)
                time.sleep(1)
            except Exception as e:
                print("Got to the end")
                break

        all_season_matches = self.driver.find_elements(By.CLASS_NAME, "event__match")
        seasons_match_details = []
        
        #Not sure why, but saved_count+3 seems to be the index of the next row not saved yet
        if saved_count > 0: saved_count += 3
        for element in all_season_matches[saved_count:]:
            link = element.find_element(By.TAG_NAME, "a").get_attribute("href")
            date = element.find_element(By.CLASS_NAME, "event__time").text.split()[0]
            home = element.find_element(By.CLASS_NAME, "event__participant--home").text
            away = element.find_element(By.CLASS_NAME, "event__participant--away").text
            home_fulltime_score = element.find_element(By.CLASS_NAME, "event__score--home").text
            away_fulltime_score = element.find_element(By.CLASS_NAME, "event__score--away").text

            # home_quater_scores = [scores.text for scores in element.find_elements(By.CLASS_NAME, "event__part--home")]
            # away_quater_scores = [scores.text for scores in element.find_elements(By.CLASS_NAME, "event__part--away")]

            if home_fulltime_score == "" or away_fulltime_score == "" : continue

            # home_q1_score = home_quater_scores[0]
            # home_q2_score = home_quater_scores[1]
            # home_q3_score = home_quater_scores[2]
            # home_q4_score = home_quater_scores[3]
            # home_ot_score = home_quater_scores[4] if len(home_quater_scores) > 4 else 0
            # away_q1_score = away_quater_scores[0]
            # away_q2_score = away_quater_scores[1]
            # away_q3_score = away_quater_scores[2]
            # away_q4_score = away_quater_scores[3]
            # away_ot_score = away_quater_scores[4] if len(away_quater_scores) > 4 else 0
            details = {
                "link": link,
                "DATE": date,
                "HOME": home,
                "AWAY": away,
                "H_SCORE": home_fulltime_score,
                # "home_q1_score": home_q1_score,
                # "home_q2_score": home_q2_score,
                # "home_q3_score": home_q3_score,
                # "home_q4_score": home_q4_score,
                # "home_ot_score": home_ot_score,
                "A_SCORE": away_fulltime_score,
                # "away_q1_score": away_q1_score,
                # "away_q2_score": away_q2_score,
                # "away_q3_score": away_q3_score,
                # "away_q4_score": away_q4_score,
                # "away_ot_score": away_ot_score,
            }
            seasons_match_details.append(details)
        
        return seasons_match_details, len(all_season_matches)


    def get_match_statistics(self, link: str):
        alt_name = {
            "Field Goals Attempted": "FGA",
            "Field Goals Made": "FG",
            "Field Goals %": "FG%",
            "2-Point Field G. Attempted": "2FGA",
            "2-Point Field Goals Made": "2FG",
            "2-Point Field Goals %": "2FG%",
            "3-Point Field G. Attempted": "3FGA",
            "3-Point Field Goals Made": "3FG",
            "3-Point Field Goals %": "3FG%",
            "Free Throws Attempted": "FTA",
            "Free Throws Made": "FT",
            "Free Throws %": "FT%",
            "Offensive Rebounds": "OREB",
            "Defensive Rebounds": "DREB",
            "Total Rebounds": "TREB",
            "Assists": "AST",
            "Blocks": "BLKS",
            "Turnovers": "TOV",
            "Steals": "STL",
            "Personal Fouls": "P_FOULS"
        }

        self.stats_driver = self.setup_driver()

        try:
            result_dict = {}

            self.stats_driver.get(link)
            with suppress(Exception):
                accept_button = WebDriverWait(self.stats_driver, 5).until(
                    EC.element_to_be_clickable((By.ID, "onetrust-accept-btn-handler"))
                )
                accept_button.click()
            time.sleep(3)
            buttons = self.stats_driver.find_elements(By.CSS_SELECTOR, "div.filterOver.filterOver--indent > div > a")
            # print(f"the buttons are {len(buttons)}")

            stats_link = buttons[2].get_attribute("href")
            # print(f"stats link = {stats_link}")
            self.stats_driver.get(stats_link)
            time.sleep(3)
            statistic_rows = self.stats_driver.find_elements(By.CLASS_NAME, "wcl-row_2oCpS")


            for row in statistic_rows:
                home_value_element = row.find_element(By.CSS_SELECTOR, ".wcl-homeValue_3Q-7P strong")
                category_element = row.find_element(By.CSS_SELECTOR, ".wcl-category_6sT1J strong")
                away_value_element = row.find_element(By.CSS_SELECTOR, ".wcl-awayValue_Y-QR1 strong")
                
                home_value = home_value_element.text
                away_value = away_value_element.text
                category = category_element.text
                if category.endswith('%'):
                    home_value = home_value.replace('%', '')
                    away_value = away_value.replace('%', '')
                if category == 'Technical Fouls': continue
                home_stats_name = "H_" + alt_name[category]
                away_stats_name = "A_" + alt_name[category]
                result_dict.update({home_stats_name : home_value})
                result_dict.update({away_stats_name : away_value})
                
        except Exception as e:
            print(f"Error: {e}")
        else:
            self.stats_driver.close()
            return result_dict

        return {}

        
    def close(self) -> None:
        if self.driver:
            self.driver.quit()


def date_compare_less(date1, date2, year):
    date1_arr = date1.split('.')
    date2_arr = date2.split('.')
    d1_d, d1_m = int(date1_arr[0]), int(date1_arr[1])
    d2_d, d2_m = int(date2_arr[0]), int(date2_arr[1])
    year = int(year)
    d1_y = year if d1_m > 8 else year+1
    d2_y = year if d2_m > 8 else year+1
    print(datetime(d1_y, d1_m, d1_d), " ", datetime(d2_y, d2_m, d2_d))
    return datetime(d1_y, d1_m, d1_d) < datetime(d2_y, d2_m, d2_d)



def season_scraper(link: str, season_text: str, folder: str, field_names: List[str]):
    try:
        scraper = FlashscoreBasketballScraper(headless=True)
        scraper.driver = scraper.setup_driver()
        yr1, yr2 = season_text.split('/')[0], season_text.split('/')[1]
        
        csv_file = os.path.join(folder, f"{yr1}-{yr2}.csv")
        file_exists = os.path.exists(csv_file)
        saved_count = 0
        empty_count = 0

        if file_exists:
            with open(csv_file, 'r', newline='', encoding='utf-8') as file:
                reader = csv.DictReader(file)
                for row in reader:
                    saved_count += 1
        print("saved count = ", saved_count)
        #exit(1)

        with open(csv_file, 'a', newline='', encoding='utf-8') as file:
            writer = csv.DictWriter(file, field_names)

            if not file_exists:
                writer.writeheader()
                file.flush()
                os.fsync(file.fileno())

            season_matches, total_count = scraper.get_season_matches(link, saved_count)
            
            counter = saved_count
            for match in season_matches:
                print(f'{counter + 1}/{total_count} \r', end='')
                
                match_stats = scraper.get_match_statistics(match["link"])                
                
                if not match_stats:
                    match_stats = scraper.get_match_statistics(match["link"])
                    if not match_stats:
                        print("ERROR: Network is disconnected?!!")
                        return
                        
                del match["link"]
                csv_row = {}


                home_possession = float(match_stats["H_FGA"]) + 0.44 * float(match_stats["H_FTA"]) - float(match_stats["H_OREB"]) + float(match_stats["H_TOV"])
                away_possession = float(match_stats["A_FGA"]) + 0.44 * float(match_stats["A_FTA"]) - float(match_stats["A_OREB"]) + float(match_stats["A_TOV"])
                
                h_off_rating = str(round((float(match["H_SCORE"]) / home_possession) * 100, 2))
                a_off_rating = str(round((float(match["A_SCORE"]) / away_possession) * 100, 2))
                h_def_rating = str(round((float(match["A_SCORE"]) / home_possession) * 100, 2))
                a_def_rating = str(round((float(match["H_SCORE"]) / away_possession) * 100, 2))
                total_score = str(float(match["H_SCORE"]) + float(match["A_SCORE"]))

                csv_row.update(match)
                csv_row.update(match_stats)
                csv_row.update({
                    "H_OFF_RATING": h_off_rating,
                    "A_OFF_RATING": a_off_rating,
                    "H_DEF_RATING": h_def_rating,
                    "A_DEF_RATING": a_def_rating,
                    "TOTAL": total_score
                })
                
                print(csv_row)
                writer.writerow(csv_row)
                file.flush()
                os.fsync(file.fileno())
                counter += 1
    
    except Exception as e:
        print(f"An error occurred: {e}")
        
    finally:
        scraper.close()



def main():
    scraper = FlashscoreBasketballScraper(headless=True)
    path = r"C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Basketball"
    
    try:
        field_names = [
                    "DATE",
                    "HOME", "AWAY", 
                    "H_SCORE", "A_SCORE", 
                    "H_FGA", "A_FGA",
                    "H_FG", "A_FG",
                    "H_FG%", "A_FG%",
                    "H_2FGA", "A_2FGA",
                    "H_2FG", "A_2FG",
                    "H_2FG%", "A_2FG%",
                    "H_3FGA", "A_3FGA",
                    "H_3FG", "A_3FG",
                    "H_3FG%", "A_3FG%",
                    "H_FTA", "A_FTA",
                    "H_FT", "A_FT",
                    "H_FT%", "A_FT%",
                    "H_OREB", "A_OREB",
                    "H_DREB", "A_DREB",
                    "H_TREB", "A_TREB",
                    "H_AST", "A_AST",
                    "H_BLKS", "A_BLKS",
                    "H_TOV", "A_TOV",
                    "H_STL", "A_STL",
                    "H_P_FOULS", "A_P_FOULS",
                    "H_OFF_RATING", "A_OFF_RATING",
                    "H_DEF_RATING", "A_DEF_RATING",
                    "TOTAL"
                ]
        
        country = "usa"
        league = "nba"
        seasons = ["2024/2025"] #, "2023/2024", "2022/2023", "2021/2022", "2021/2020"]
        folder = os.path.join(path, league)
        os.makedirs(folder, exist_ok=True)

        available_seasons, links = scraper.get_seasons_list(country, league)
        threads = []

        for season in seasons:
            for season_text, link in zip(available_seasons, links):
                if season in season_text:
                    worker = Thread(target=season_scraper, args=(link, season, folder, field_names,))
                    threads.append(worker)
                    break
            else:
                print("No matching seasons found")

        for thread in threads:
            thread.start()

        for thread in threads:
            thread.join()
        
    except KeyboardInterrupt:
        print("\nScraping interrupted by user")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        scraper.close()

if __name__ == "__main__":
    main()