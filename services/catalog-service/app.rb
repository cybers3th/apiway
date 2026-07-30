# Catalog microservice. Runs behind the gateway, which handles authentication
# (JWT or API key) and passes the resolved identity in X-Auth-* headers.

require 'sinatra/base'
require 'json'

class CatalogService < Sinatra::Base
  set :bind, '0.0.0.0'
  set :port, 9200
  set :show_exceptions, false

  CATALOG = [
    { id: 1, sku: 'WID-001', name: 'Widget',  price_cents: 1299, stock: 42 },
    { id: 2, sku: 'GAD-002', name: 'Gadget',  price_cents: 4599, stock: 7  },
    { id: 3, sku: 'GIZ-003', name: 'Gizmo',   price_cents: 899,  stock: 0  }
  ].freeze

  before do
    content_type :json
  end

  helpers do
    def caller_name
      request.env['HTTP_X_AUTH_USERNAME'] || 'unknown'
    end
  end

  get '/healthz' do
    { status: 'ok' }.to_json
  end

  get '/api/catalog' do
    { caller: caller_name, items: CATALOG }.to_json
  end

  get '/api/catalog/:id' do
    item = CATALOG.find { |i| i[:id] == params[:id].to_i }
    halt 404, { error: 'item not found', status: 404 }.to_json unless item
    item.to_json
  end

  run! if app_file == $PROGRAM_NAME
end
