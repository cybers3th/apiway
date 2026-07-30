// Orders microservice. It runs behind the gateway, which authenticates callers
// and forwards the resolved identity in X-Auth-* headers. The service trusts
// those headers and does no token parsing of its own.

using System.Collections.Concurrent;

var builder = WebApplication.CreateBuilder(args);
var app = builder.Build();

var orders = new ConcurrentDictionary<int, Order>();
var nextId = 0;
foreach (var seed in new[]
{
    new Order(0, "widget", 3, "acme"),
    new Order(0, "gadget", 1, "globex"),
})
{
    var id = Interlocked.Increment(ref nextId);
    orders[id] = seed with { Id = id };
}

string Caller(HttpContext ctx) =>
    ctx.Request.Headers["X-Auth-Username"].FirstOrDefault() ?? "unknown";

app.MapGet("/healthz", () => Results.Json(new { status = "ok" }));

app.MapGet("/api/orders", (HttpContext ctx) =>
    Results.Json(new
    {
        caller = Caller(ctx),
        orders = orders.Values.OrderBy(o => o.Id),
    }));

app.MapGet("/api/orders/{id:int}", (int id) =>
    orders.TryGetValue(id, out var order)
        ? Results.Json(order)
        : Results.NotFound(new { error = "order not found", status = 404 }));

app.MapPost("/api/orders", async (HttpContext ctx, NewOrder input) =>
{
    if (string.IsNullOrWhiteSpace(input.Item) || input.Quantity <= 0)
        return Results.BadRequest(new { error = "item and positive quantity required" });

    var id = Interlocked.Increment(ref nextId);
    var order = new Order(id, input.Item, input.Quantity, Caller(ctx));
    orders[id] = order;
    await Task.CompletedTask;
    return Results.Created($"/api/orders/{id}", order);
});

app.Run("http://0.0.0.0:9100");

record Order(int Id, string Item, int Quantity, string CreatedBy);
record NewOrder(string Item, int Quantity);
