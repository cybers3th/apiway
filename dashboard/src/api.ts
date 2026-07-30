// Thin client for the Apiway gateway. All calls go through the /gw prefix,
// which nginx (prod) or Vite (dev) proxies to the gateway.
const BASE = (import.meta.env.VITE_GATEWAY_BASE as string | undefined) ?? "/gw";

export interface LoginResponse {
  access_token: string;
  token_type: string;
  expires_in: number;
  roles: string[];
  permissions: string[];
}

export interface RouteInfo {
  path_prefix: string;
  upstream: string;
  auth: boolean;
  allow_api_key: boolean;
  strip_prefix: boolean;
  required_permissions: string[];
  method_permissions: Record<string, string[]>;
}

async function errorMessage(res: Response, fallback: string): Promise<string> {
  try {
    const data = await res.json();
    return (data && data.error) || fallback;
  } catch {
    return fallback;
  }
}

export async function login(username: string, password: string): Promise<LoginResponse> {
  const res = await fetch(`${BASE}/auth/login`, {
    method: "POST",
    headers: { "content-type": "application/json" },
    body: JSON.stringify({ username, password }),
  });
  if (!res.ok) throw new Error(await errorMessage(res, `login failed (${res.status})`));
  return res.json();
}

export async function fetchRoutes(token: string): Promise<RouteInfo[]> {
  const res = await fetch(`${BASE}/admin/routes`, {
    headers: { authorization: `Bearer ${token}` },
  });
  if (!res.ok) throw new Error(await errorMessage(res, `could not load routes (${res.status})`));
  const data = await res.json();
  return data.routes as RouteInfo[];
}

export interface ProbeResult {
  status: number;
  ok: boolean;
  body: string;
}

export async function probe(token: string | null, path: string): Promise<ProbeResult> {
  const headers: Record<string, string> = {};
  if (token) headers.authorization = `Bearer ${token}`;
  const res = await fetch(`${BASE}${path}`, { headers });
  const body = await res.text();
  return { status: res.status, ok: res.ok, body };
}

export function decodeJwt(token: string): Record<string, unknown> | null {
  try {
    const payload = token.split(".")[1];
    const json = atob(payload.replace(/-/g, "+").replace(/_/g, "/"));
    return JSON.parse(json);
  } catch {
    return null;
  }
}
