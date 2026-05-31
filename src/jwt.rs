use chrono::{DateTime, Duration, Months, Utc};
use data_encoding::BASE64;
use hyper::StatusCode;
use jsonwebtoken::{Algorithm, DecodingKey, EncodingKey, Header, Validation, decode, encode};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

use super::{HttpError, Result};

pub struct Jwt {
    secret: Vec<u8>,
}

impl Jwt {
    pub fn new(key: &str) -> Result<Self> {
        Ok(Self {
            secret: BASE64.decode(key.as_bytes())?,
        })
    }

    pub fn by_years(&self, issuer: &str, subject: &str, years: u8) -> Result<String> {
        let now = Utc::now();
        self.build(
            issuer,
            subject,
            now,
            now - Duration::seconds(1),
            now.checked_add_months(Months::new((years as u32) * 12))
                .ok_or_else(|| Box::new(HttpError(StatusCode::BAD_REQUEST, None)))?,
        )
    }
    pub fn by_ttl(&self, issuer: &str, subject: &str, ttl: Duration) -> Result<String> {
        let now = Utc::now();
        self.build(issuer, subject, now, now - Duration::seconds(1), now + ttl)
    }
    fn build(
        &self,
        issuer: &str,
        subject: &str,
        issued_at: DateTime<Utc>,
        not_before: DateTime<Utc>,
        expires_at: DateTime<Utc>,
    ) -> Result<String> {
        let mut header = Header::new(Algorithm::HS512);
        header.kid = Some(Uuid::new_v4().to_string());

        let token = encode(
            &header,
            &Claims {
                iss: issuer.to_string(),
                sub: subject.to_string(),
                iat: issued_at.timestamp() as usize,
                nbf: not_before.timestamp() as usize,
                exp: expires_at.timestamp() as usize,
            },
            &EncodingKey::from_secret(&self.secret),
        )?;
        Ok(token)
    }
    pub fn verify(&self, token: &str, issuer: &str) -> Result<String> {
        let mut validation = Validation::new(Algorithm::HS512);
        validation.set_issuer(&[issuer]);
        let claims =
            decode::<Claims>(&token, &DecodingKey::from_secret(&self.secret), &validation)?.claims;
        Ok(claims.sub)
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct Claims {
    iss: String,
    sub: String,
    exp: usize,
    iat: usize,
    nbf: usize,
}
