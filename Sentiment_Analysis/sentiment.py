from urllib.request import urlopen, Request
from bs4 import BeautifulSoup
from nltk.sentiment.vader import SentimentIntensityAnalyzer
import pandas as pd
import matplotlib.pyplot as plt

# Concating the url to retrieve the HTML page of 4 stock tickets
finviz_url = 'https://finviz.com/quote.ashx?t='
tickers = ['AFG', 'ALL', 'CINF', 'CNA']

# A dictionary where the key is the stock ticker and the value is the corresponding HTML page
news_tables = {}

# For each ticker, concatenate the url, request the HTM page, parse it, and put into news_tables
for ticker in tickers:
    url = finviz_url + ticker

    req = Request(url=url, headers={'user-agent': 'my-app'})
    response = urlopen(req)

    html = BeautifulSoup(response, features='html.parser')
    news_table = html.find(id='news-table')
    news_tables[ticker] = news_table
    
AFG_data = []
ALL_data = []
CINF_data = []
CNA_data = []

# Formatting the body data to retrieve only the news headlines, 
# which will be used for sentiment analysis

for ticker, news_table in news_tables.items():
    for row in news_table.findAll('tr'):
        title = row.a.text
        date_data = row.td.text.split(' ')

        if len(date_data) == 1:
            time = date_data[0]
        else:
            date = date_data[0]
            time = date_data[1]
            
        if ticker == 'AFG':
            AFG_data.append([ticker, date, time, title])
        elif ticker == 'ALL':
            ALL_data.append([ticker, date, time, title])
        elif ticker == 'CINF':
            CINF_data.append([ticker, date, time, title])
        else:
            CNA_data.append([ticker, date, time, title])

datas = [AFG_data, ALL_data, CINF_data, CNA_data]  

# Some column have inconsistent date/time format so more data cleaning is needed
# Also, visualizing the final result after applying sentiment analysis with the help of a library (vader)

for data in datas:
    df = pd.DataFrame(data, columns=['ticker', 'date', 'time', 'title'])
    df['date'] = pd.to_datetime(df.date).dt.date
    
    vader = SentimentIntensityAnalyzer()
    
    f = lambda title: vader.polarity_scores(title)['compound']
    df['compound'] = df['title'].apply(f)
    
    plt.figure(figsize=(12,8))
    
    plt.plot(df['date'], df['compound'])


