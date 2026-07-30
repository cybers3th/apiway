FROM ruby:3.3-slim
WORKDIR /app
COPY Gemfile ./
RUN gem install bundler && bundle install
COPY app.rb ./
EXPOSE 9200
CMD ["ruby", "app.rb"]
