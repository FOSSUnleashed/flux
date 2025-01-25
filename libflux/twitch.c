#include <new.h>
#include <flux/twitch.h>
#include <string.h>

Twitch *alloc_twitch() {
	Twitch *t;

	t = calloc(1, sizeof(Twitch));

	if (NULL == t) {
		goto end;
	}

	configInit(&t->cfg, NULL);

	configAssignString(&t->cfg, "client_id", t->clientId, TWITCH_MAX_SIZE, NULL);
	configAssignString(&t->cfg, "client_secret", t->clientSecret, TWITCH_MAX_SIZE, NULL);
	configAssignString(&t->cfg, "user_token", t->userToken, TWITCH_MAX_SIZE, NULL);
	configAssignString(&t->cfg, "app_token", t->appToken, TWITCH_MAX_SIZE, NULL);
	configAssignString(&t->cfg, "user_id", t->userId, TWITCH_MAX_SIZE, NULL);

	end:
	return t;
}

Twitch *flux_twitch_create() {
	Twitch *t;

	t = alloc_twitch();

	if (NULL == t) {
		goto end;
	}

	t->ssl = 1;
	strcpy(t->base, "/helix/");
	strcpy(t->host, "twitch.tv");

	if (-1 == ipaddr_remote4(&t->addr, "twitch.tv", 443, -1)) {
		free(t);
		t = NULL;
		goto end;
	}

	end:
	return t;
}

Twitch *flux_twitch_mock_create(const char * host, uint32_t port) {
	Twitch *t;

	t = alloc_twitch();

	if (NULL == t) {
		goto end;
	}

	strcpy(t->base, "/mock/");
	strcpy(t->host, host);

	if (-1 == ipaddr_remote4(&t->addr, host, port, -1)) {
		free(t);
		t = NULL;
		goto end;
	}

	end:
	return t;
}

/*

Twitch Object:
{
	Tokens[3]
	Client ID
	Client Secret
	Broadcaster ID (not needed for any of the authentication)
	Scopes (hard code for now)
}

////// === App access token === //////

POST TO: https://id.twitch.tv/oauth2/token

"Content-Type", "x-www-form-urlencoded" (3 fields)

client_id=hof5gwx0su6owfnys0yan9c87zr6t
&client_secret=41vpdji4e9gif29md0ouet6fktd2
&grant_type=client_credentials

{
  "access_token": "jostpf5q0uzmxmkba9iyug38kjtgh",
  "expires_in": 5011271,
  "token_type": "bearer"
}

////// === User Access Token === ///////

Click link from index.html

Get code from redirect to:

GOOD: http://localhost:3000/?code=gulfwdmys5lsm6qyz4xiz9q32l10&scope=channel%3Amanage%3Apolls+channel%3Aread%3Apolls&state=c3ab8aa609ea11e793ae92361f002671

BAD: http://localhost:3000/?error=access_denied&error_description=The+user+denied+you+access&state=c3ab8aa609ea11e793ae92361f002671

POST TO: https://id.twitch.tv/oauth2/token (5 fields)

client_id=hof5gwx0su6owfnys0yan9c87zr6t
&client_secret=41vpdji4e9gif29md0ouet6fktd2
&grant_type=authorization_code
&code=gulfwdmys5lsm6qyz4xiz9q32l10 ### CODE ###
&redirect_uri=http://localhost:3009

{
  "access_token": "rfx2uswqe8l4g1mkagrvg5tv0ks3",
  "expires_in": 14124,
  "refresh_token": "5b93chm6hdve3mycz05zfzatkfdenfspp1h1ar2xxdalen01",
  "scope": [
    "channel:moderate",
    "chat:edit",
    "chat:read"
  ],
  "token_type": "bearer"
}

////// ==== Refresh User Access Tokens ==== ////

POST TO: https://id.twitch.tv/oauth2/token (4 fields)

curl -X POST https://id.twitch.tv/oauth2/token \
-H 'Content-Type: application/x-www-form-urlencoded' \
-d 'grant_type=refresh_token&refresh_token=gdw3k62zpqi0kw01escg7zgbdhtxi6hm0155tiwcztxczkx17&client_id=<your client id goes here>&client_secret=<your client secret goes here>'

client_id=hof5gwx0su6owfnys0yan9c87zr6t
&client_secret=41vpdji4e9gif29md0ouet6fktd2
&grant_type=refresh_token
&refresh_token=...

{
  "access_token": "1ssjqsqfy6bads1ws7m03gras79zfr",
  "refresh_token": "eyJfMzUtNDU0OC4MWYwLTQ5MDY5ODY4NGNlMSJ9%asdfasdf=",
  "scope": [
    "channel:read:subscriptions",
    "channel:manage:polls"
  ],
  "token_type": "bearer"
}

// No expiration???!?!?!?!

// log json to file

token object:

{
	buffer[1024] ? data bits
	Bit : Active ? istead of type?
	type ? APP | REFRESH | USER | INACTIVE
	expiration ? flux_s() + expires_in
}

// */
